#include "Search.h"

#include <iostream>

#include <cassert>
#include <vector>

#include "../Stats.h"
#include "../Board.h"
#include "MoveGen.h"
#include "MoveOrdering.h"
#include "Evaluation.h"
#include "../Psqt.h"

constexpr int WINDOW_MIN_DEPTH = 5;
constexpr int WINDOW_SIZE = 15;

constexpr int REVERSE_FUTILITY_MAX_DEPTH = 6;
constexpr int REVERSE_FUTILITY_MARGIN = 220;

constexpr int FUTILITY_QUIESCENCE_MARGIN = 100;
constexpr int FUTILITY_MARGIN = 160;
constexpr int FUTILITY_MAX_DEPTH = 6;

static thread_local Thread *_thread = nullptr;

auto &threadInfo() { return *_thread; }

Settings Search::_searchSettings{ MAX_DEPTH, 1, 16, true };
TranspositionTable Search::_transpTable{ _searchSettings.tableSizeMb() };
Search::SharedState Search::_sharedState{};

void Search::clearAll()
{
	_transpTable.clear();
}

void Search::stopSearch()
{
	_sharedState.stopped = true;
}

bool Search::setTableSize(const std::size_t sizeMb)
{
	return _transpTable.setSize(sizeMb);
}

Move Search::findBestMove(Board board, const Settings &settings)
{
	Stats::resetStats();
	// Apply Settings
	_searchSettings = settings;
	const auto threadCount = settings.threadCount();

	_transpTable.incrementAge();
	if (settings.tableSizeMb() != 0)
		setTableSize(settings.tableSizeMb());

	board.ply = 0;
	// Reset Depth Counter
	_sharedState.stopped = false;
	_sharedState.nodes = 0;
	_sharedState.depth = 0;
	_sharedState.bestScore = VALUE_MIN;
	_sharedState.time = 0;
	_sharedState.lastReportedDepth = 0;

	Stats::restartTimer();

	const auto work = [&, board](const i32 threadId)
	{
		assert(threadId >= 1);
		assert(threadId <= i32(threadCount));
		_thread = new Thread(threadId, threadId == 1);

		while (!_sharedState.stopped)
		{
			const auto currentDepth = i32(_sharedState.depth);
			const auto depth = currentDepth + 1 + i32(Bits::bitScanForward(u64(threadId)));

			iterativeDeepening(board, std::min<i32>(depth, _searchSettings.depth()));
		}

		// Clean Heuristics after search
		delete _thread;
	};

	std::vector<std::thread> threads;
	threads.reserve(threadCount);

	for (usize i{}; i < threadCount; ++i)
		threads.emplace_back(work, i32(i) + 1);

	for (auto &&thread : threads)
		thread.join();

	return _transpTable[board.zKey].move;
}

void Search::printUci(Board &board)
{
	if (!threadInfo().mainThread)
		return;
	std::array<Move, MAX_DEPTH> pvArray{};

	const auto getPvLine = [&board, &pvArray](const int depth) -> int
	{
		assert(depth <= MAX_DEPTH);

		int count{};

		while (true)
		{
			Move move = _transpTable[board.zKey].move;
			if (moveExists(board, move))
			{
				board.makeMove(move);
				pvArray[count++] = move;
			} else
				break;

			if (move.empty() || count >= depth)
				break;
		}

		while (board.ply > 0)
			board.undoMove();

		return count;
	};

	int depth;
	int bestScore;
	size_t time;

	{
		std::lock_guard lock{ _sharedState.mutex };
		depth = _sharedState.depth;
		bestScore = _sharedState.bestScore;
		time = _sharedState.time;
	}

	const int lastReportedDepth = _sharedState.lastReportedDepth;
	if (!_sharedState.stopped && depth > lastReportedDepth)
	{
		const int pvMoves = getPvLine(depth);

		const int cp = bestScore * 100 / 213;
		std::cout << "info depth " << depth << " score cp " << cp
				  << " nodes " << _sharedState.nodes << " time " << time;

		std::cout << " pv: ";
		for (int pvCount = 0; pvCount < pvMoves; ++pvCount)
			std::cout << pvArray[pvCount].toString() << ' ';

		std::cout << std::endl;

		_sharedState.lastReportedDepth = depth;
		_sharedState.lastReportedBestMove = pvArray[0];

		const auto elapsedTime = Stats::getElapsedMs();
		const auto timeLeft = _searchSettings.searchTime() - elapsedTime;

		if (depth >= _searchSettings.depth() || timeLeft < elapsedTime / 2)
			stopSearch();
	}

	if (_sharedState.stopped)
		std::cout << "bestmove " << _sharedState.lastReportedBestMove.toString() << std::endl;
}

void Search::iterativeDeepening(Board board, const int targetDepth)
{
	auto &&thread = threadInfo();
	int bestScore = VALUE_MIN;

	for (int currentDepth = 1; currentDepth <= targetDepth; ++currentDepth)
	{
		// Stop if we have found a mate value
		if (bestScore > VALUE_MATE_MAX_DEPTH /*&& !bestMove.empty()*/)
			break;

		thread.nodesCount = 0;

		bestScore = aspirationWindow(board, currentDepth, bestScore);
		_sharedState.nodes += thread.nodesCount;

		if (bestScore != VALUE_MIN && currentDepth > _sharedState.depth)
		{
			std::lock_guard lock{ _sharedState.mutex };
			if (currentDepth > _sharedState.depth)
			{
				_sharedState.depth = currentDepth;
				_sharedState.bestScore = bestScore;
				_sharedState.time = Stats::getElapsedMs();
			}
		}

		if (thread.mainThread)
			printUci(board);

		if (_sharedState.stopped)
			break;
	}
}

int Search::aspirationWindow(Board &board, const int depth, const int bestScore)
{
	int alpha = VALUE_MIN;
	int beta = VALUE_MAX;
	int delta = WINDOW_SIZE;

	if (depth >= WINDOW_MIN_DEPTH)
	{
		alpha = std::max<int>(VALUE_MIN, bestScore - delta);
		beta = std::min<int>(VALUE_MAX, bestScore + delta);
	}

	int adjustedDepth = depth;

	while (true)
	{
		const int searchedValue = search<true>(board, alpha, beta, adjustedDepth, true, true);
		if (_sharedState.stopped)
			return 0;

		if (searchedValue <= alpha)
		{
			beta = (alpha + beta) / 2;
			alpha = std::max<int>(VALUE_MIN, alpha - delta);
			adjustedDepth = depth;
		} else if (searchedValue >= beta)
		{
			beta = std::min<int>(VALUE_MAX, beta + delta);
			adjustedDepth -= (abs(searchedValue) <= VALUE_MAX / 2);
		} else
		{
			// If the search returned a result within our window, return the value
			return searchedValue;
		}

		// Expand the search Window
		delta += delta / 2;
		assert(alpha >= VALUE_MIN && beta <= VALUE_MAX);
	}
}

template<bool PvNode>
int Search::search(Board &board, int alpha, int beta, const int depth,
				   const bool doNull, const bool doLmr)
{
	const bool rootNode = !board.ply;
	if (rootNode)
		assert(PvNode);

	if (checkTimeAndStop())
		return 0;

	++threadInfo().nodesCount;

	if (!rootNode)
	{
		if (board.isDrawn())
			return 0;

		if (board.ply >= MAX_DEPTH)
			return Evaluation::invertedValue(board);
	}

	if (depth <= 0)
		return _searchSettings.doQuietSearch()
			   ? searchCaptures(board, alpha, beta, depth)
			   : Evaluation::invertedValue(board);

	const int originalAlpha = alpha;
	const int startPly = board.ply;
	auto &&thread = threadInfo();

	// Prove the Transposition Table
	if (const SearchEntry entry = _transpTable[board.zKey];
		!entry.qSearch
		&& entry.age == _transpTable.currentAge()
		&& entry.key == board.zKey
		&& entry.depth >= depth)
	{
		const int entryScore = entry.move.getScore();

		if (entry.flag == SearchEntry::Flag::EXACT)
			return entryScore;
		if (entry.flag == SearchEntry::Flag::ALPHA)
			alpha = std::max(alpha, entryScore);
		else if (entry.flag == SearchEntry::Flag::BETA)
			beta = std::min(beta, entryScore);

		if (alpha >= beta)
			return entryScore;
	}

	const int eval = Evaluation::invertedValue(board);
	const bool nodeInCheck = board.isSideInCheck();
	thread.eval[board.ply] = eval;

	// We are improving if our static eval increased in the last move
	const bool improving = startPly > 1 && (eval > thread.eval[startPly - 2]);
	const int futilityMarginEval = eval + FUTILITY_MARGIN * depth;

	// Reverse Futility Pruning
	if (!PvNode
		&& !nodeInCheck
		&& depth <= REVERSE_FUTILITY_MAX_DEPTH
		&& (eval - REVERSE_FUTILITY_MARGIN * std::max(1, depth - improving)) >= beta)
		return eval;

	// Null Move Pruning
	if (!PvNode
		&& doNull
		&& depth >= 4
		&& eval >= beta
		&& !nodeInCheck
		&& board.pieceCount[Piece{ QUEEN, board.colorToMove }]
		   + board.pieceCount[Piece{ ROOK, board.colorToMove }] > 0)
	{
		board.makeNullMove();
		const int nullScore = -search<false>(board, -beta, -beta + 1, depth - 4, false, false);
		board.undoNullMove();

		if (nullScore >= beta)
		{
			Stats::incNullCuts();
			return beta;
		}
	}

	MoveList moveList(board);
	MoveOrdering::sortMoves(thread, board, moveList);

	size_t legalCount{};
	size_t searchedCount{};
	int bestScore = Value::VALUE_MIN;
	Move bestMove;

	while (!moveList.empty())
	{
		assert(startPly == board.ply);

		const Move move = MoveOrdering::getNextMove(moveList);
		const bool pvMove = move.getScore() == MoveOrdering::PV_SCORE;

		if (!board.makeMove(move))
			continue;
		++legalCount;

		// Futility Pruning
		if (depth < FUTILITY_MAX_DEPTH
			&& !pvMove
			&& searchedCount > 3
			&& futilityMarginEval <= alpha
			&& !isMateValue(static_cast<Value>(alpha))
			&& !move.isTactical()
			&& !(move.flags().doublePawnPush())
			&& !move.isAdvancedPawnPush()
			&& !nodeInCheck
			&& !board.isSideInCheck())
		{
			Stats::incFutilityCuts();
			if (futilityMarginEval > bestScore)
				bestScore = eval;
			board.undoMove();
			continue;
		}

		int moveScore = alpha;
		bool doFullSearch = true;

		// Late Move Reductions
		if (!pvMove
			&& doNull
			&& doLmr
			&& searchedCount > 3
			&& depth >= 3
			&& board.ply > 4
			&& !move.isTactical()
			&& !nodeInCheck
			&& !board.isSideInCheck())
		{
			moveScore = -search<false>(board, -alpha - 1, -alpha, depth - 2, true, false);

			// Search with a full window if LMR failed high
			doFullSearch = moveScore > alpha;
			if (!doFullSearch)
				Stats::incLmrCount();
		}

		if (doFullSearch)
		{
			if (pvMove)
				moveScore = -search<true>(board, -beta, -alpha, depth - 1, true, true);
			else
				moveScore = -search<false>(board, -beta, -alpha, depth - 1, true, true);
		}

		++searchedCount;
		const int searchedPly = board.ply;
		board.undoMove();

		// Mate Pruning
		if (moveScore == Value::VALUE_MAX - searchedPly) // Winning
		{
			if (moveScore < beta)
			{
				beta = moveScore;
				if (alpha >= moveScore)
					return moveScore;
			}
		} else if (moveScore == Value::VALUE_MIN + searchedPly) // Losing
		{
			if (moveScore > alpha)
			{
				alpha = moveScore;
				if (beta <= moveScore)
					return moveScore;
			}
		}

		if (moveScore > bestScore)
		{
			bestScore = moveScore;
			bestMove = move;

			if (bestScore > alpha)
			{
				alpha = bestScore;
				thread.history[move.from()][move.to()] += depth;
			}
		}

		// Alpha-Beta Pruning
		if (bestScore >= beta)
		{
			if (!move.isTactical())
			{
				auto &searchKillers = thread.killers;
				searchKillers[1][startPly] = searchKillers[0][startPly];
				searchKillers[0][startPly] = move.getContents();
			}

			break;
		}
	}

	if (legalCount == 0)
		return board.isSideInCheck() ? Value::VALUE_MIN + startPly : 0;

	Stats::incNodesSearched(searchedCount);

	bestMove.setScore(bestScore);
	storeTtEntry(bestMove, board.zKey, alpha, originalAlpha, beta, depth, false);

	assert(abs(bestScore) != VALUE_MIN);
	return alpha;
}

int Search::searchCaptures(Board &board, int alpha, int beta, const int depth)
{
	if (checkTimeAndStop())
		return 0;

	if (board.isDrawn())
		return 0;

	++threadInfo().nodesCount;

	const short startPly = board.ply;

	if (startPly >= MAX_DEPTH)
		return Evaluation::invertedValue(board);

	const bool nodeInCheck = board.isSideInCheck();

	const int originalAlpha = alpha;
	int standPat = Value::VALUE_MIN;
	int bestScore = Value::VALUE_MIN;
	Move bestMove;

	if (!nodeInCheck)
	{
		if (const SearchEntry entry = _transpTable[board.zKey];
			entry.age == _transpTable.currentAge()
			&& entry.key == board.zKey
			&& entry.depth >= depth)
		{
			const int entryScore = entry.move.getScore();

			if (entry.flag == SearchEntry::Flag::EXACT)
				return entry.move.getScore();
			if (entry.flag == SearchEntry::Flag::ALPHA)
				alpha = std::max(alpha, entryScore);
			else if (entry.flag == SearchEntry::Flag::BETA)
				beta = std::min(beta, entryScore);

			if (alpha >= beta)
				return entryScore;
		}

		standPat = Evaluation::invertedValue(board);

		alpha = std::max(alpha, standPat);
		if (alpha >= beta)
			return standPat;
	}

	MoveList moveList(board);

	MoveOrdering::sortQMoves(moveList);
	size_t legalCount{};
	size_t searchedCount{};

	const bool isEndGame = board.getPhase() < Phase::MIDDLE_GAME_PHASE / 3;

	while (!moveList.empty())
	{
		assert(startPly == board.ply);

		const Move move = MoveOrdering::getNextMove(moveList);

		if (!board.makeMove(move))
			continue;
		++legalCount;

		if (!nodeInCheck // Look for all check evasions
			&& !move.isTactical()
			&& searchedCount)
		{
			board.undoMove();
			break; // The moves are sorted so we can break if is not a capture
		}

		const int futilityEval = standPat + PSQT[move.piece()][move.to()].eg
								 + Evaluation::getPieceValue(move.promotedPiece())
								 + FUTILITY_QUIESCENCE_MARGIN;
		// Futility Pruning
		if (!nodeInCheck
			&& !isEndGame
			&& board.ply > 2
			&& standPat != VALUE_MIN
			&& searchedCount
			&& futilityEval <= alpha
			&& !board.isSideInCheck())
		{
			board.undoMove();
			Stats::incFutilityCuts();
			continue;
		}

		const int moveScore = -searchCaptures(board, -beta, -alpha, depth - 1);
		board.undoMove();
		++searchedCount;

		if (_sharedState.stopped)
			return 0;

		if (moveScore > bestScore)
		{
			bestScore = moveScore;
			bestMove = move;
		}

		if (bestScore >= beta)
		{
			Stats::incNodesSearched(searchedCount);
			break;
		}

		alpha = std::max(alpha, moveScore);
	}

	if (legalCount == 0)
		return board.isSideInCheck() ? Value::VALUE_MIN + board.ply : 0;

	if (!nodeInCheck)
	{
		bestMove.setScore(bestScore);
		storeTtEntry(bestMove, board.zKey, alpha, originalAlpha, beta, depth, true);
	}

	Stats::incNodesSearched(searchedCount);

	return bestScore;
}

inline void Search::storeTtEntry(const Move &bestMove, const u64 key, const int alpha,
								 const int originalAlpha, const int beta, const int depth,
								 const bool qSearch)
{
	// Avoid putting an empty move in the Transposition Table if alpha was not raised
	assert(!bestMove.empty());

	// Store the result in the transposition table
	auto flag = SearchEntry::Flag::EXACT;
	if (alpha <= originalAlpha)
		flag = SearchEntry::Flag::ALPHA;
	else if (bestMove.getScore() >= beta)
		flag = SearchEntry::Flag::BETA;

	_transpTable.insert({ key, bestMove, static_cast<short>(depth), flag, qSearch });
}

bool Search::checkTimeAndStop()
{
	if (threadInfo().mainThread
		&& _searchSettings.isTimeSet()
		&& (threadInfo().nodesCount & 2047u) == 0
		&& Stats::getElapsedMs() >= _searchSettings.searchTime())
		stopSearch();
	return _sharedState.stopped;
}
