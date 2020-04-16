#include "Search.h"

#include <cassert>
#include <iostream>

#include "../Stats.h"
#include "../data/Board.h"
#include "MoveGen.h"
#include "MoveOrdering.h"
#include "Evaluation.h"

std::array<std::array<Move, MAX_DEPTH>, 2> Search::_searchKillers{};
std::array<std::array<int, 64>, 64> Search::_searchHistory{};
ThreadPool Search::_threadPool(1);
TranspositionTable Search::_transpTable(2);
bool Search::_quiescenceSearchEnabled{};

/*RootMove Search::negaMaxRoot(const std::vector<RootMove> &validMoves, const unsigned jobCount, const short ply)
{
	std::vector<std::future<void>> futures;
	futures.reserve(jobCount);
	std::mutex mutex;
	
	RootMove bestMove = validMoves.front();
	auto currentMove = validMoves.begin();
	const auto lastMove = validMoves.end();
	short alpha = VALUE_MIN;

	const auto worker = [&] {
		mutex.lock();

		while (currentMove != lastMove)
		{
			// Make a copy of the needed variables while locked
			const RootMove &move = *currentMove;
			++currentMove;
			const short beta = -alpha;

			mutex.unlock(); // Process the result asynchronously
			const short result = -negaMax(move.board, ply, VALUE_MIN, beta, 1, false);
			mutex.lock();

			if (result > alpha)
			{
				alpha = result;
				bestMove = move;
			}
		}

		mutex.unlock();
	};

	// Initially create only use thread
	futures.emplace_back(_threadPool.submitTask(worker));

	if (jobCount > 1)
	{
		// Wait until an alpha bound has been found
		while (alpha == VALUE_MIN)
			std::this_thread::yield();

		// After an alpha bound was found start searching using multiple threads
		for (unsigned i = 1u; i < jobCount; ++i)
			futures.emplace_back(_threadPool.submitTask(worker));
	}

	for (auto &future : futures)
		future.get(); // Wait for the search to finish

	s_BestMoveFound = validMoves.front().board.colorToMove ? -alpha : alpha;

	return bestMove;
}*/

Move Search::findBestMove(Board board, const Settings &settings)
{
	// Apply Settings
	int depth = settings.getBaseSearchDepth();
//	const auto threadCount = settings.getThreadCount();
	_quiescenceSearchEnabled = settings.performQuiescenceSearch();

	const size_t pawnTableSize = std::max<size_t>(settings.getCacheTableSizeMb() / 10u, 1u);
	Evaluation::getPawnTable().setSize(pawnTableSize);
	
	// If the Transposition Table wasn't resized, increment its age
	if (!_transpTable.setSize(std::max<size_t>(settings.getCacheTableSizeMb() - pawnTableSize, 2u)))
		_transpTable.incrementAge();

	// Update ThreadPool size if needed
//	if (_threadPool.getThreadCount() != threadCount)
//		_threadPool.updateThreadCount(threadCount);

	board.ply = 0;

	_searchKillers.fill({ });
	_searchHistory.fill({ });

	if (board.getPhase() == Phase::ENDING)
		++depth;

	return iterativeDeepening(board, depth);
}

Move Search::iterativeDeepening(Board &board, const int depth)
{
	std::array<Move, MAX_DEPTH> pvArray{};
	
	const auto getPvLine = [] (Board &board, std::array<Move, MAX_DEPTH> &pvArray, const int depth) -> int
	{
		assert(depth < MAX_DEPTH);
		
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

	Move bestMove;

	for (int currentDepth = 1; currentDepth <= depth; ++currentDepth)
	{
		const int bestScore = search(board, Value::VALUE_MIN, Value::VALUE_MAX, currentDepth, true, true);
		const int pvMoves = getPvLine(board, pvArray, currentDepth);
		bestMove = pvArray[0];
		
		std::cout << "Depth: " << currentDepth << ", Score: " << bestScore << ", PvMoves: ";
		for (int pvNum = 0; pvNum < pvMoves; ++pvNum)
			std::cout << pvArray[pvNum].toString() << ", ";

		std::cout << '\n';
	}

	return bestMove;
}

int Search::search(Board &board, int alpha, int beta, const int depth, const bool doNull, const bool doMoveCountPruning)
{
	if (board.ply && (board.fiftyMoveRule == 100 || board.isRepetition()))
		return 0;

	if (board.ply == MAX_DEPTH)
		return evaluate(board);
	
	// If in check and not already extended, allow the search to be extended to depth -1
	if (depth <= 0)
		return _quiescenceSearchEnabled ? searchCaptures(board, alpha, beta) : evaluate(board);

	const int originalAlpha = alpha;
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

	const int currentPly = board.ply;

	// Null Move
	const Color color = board.colorToMove;
	if (doNull && board.ply && depth >= 4 && board.pieceCount[Piece(QUEEN, color)] + board.pieceCount[Piece(ROOK, color)] > 0 && !board.isInCheck(color))
	{
		board.makeNullMove();
		const int nullScore = -search(board, -beta, -beta + 1, depth - 4, false, false);
		board.undoNullMove();

		if (nullScore >= beta)
			return beta;
	}

	MoveList moveList(board);
	MoveOrdering::sortMoves(board, moveList.begin(), moveList.end());
	Stats::incrementNodesGenerated(moveList.size());

	size_t legalCount{};
	int bestScore = Value::VALUE_MIN;
	Move bestMove{};

	for (const Move &move : moveList)
	{
		assert(currentPly == board.ply);
		if (!board.makeMove(move))
			continue;
		++legalCount;

		int moveScore = alpha;
		bool didLmr = false;

		// Late Move Reductions
		if (doNull
			&& doMoveCountPruning
			&& legalCount >= 4
			&& depth >= 3
			&& board.ply > 4
			&& !(move.flags() & Move::Flag::CAPTURE)
			&& !(move.flags() & Move::Flag::PROMOTION)
			&& !board.isInCheck(board.colorToMove))
		{
			moveScore = -search(board, -alpha - 1, -alpha,  depth - 2, false, false);
			didLmr = true;
		}

		if (!didLmr || moveScore > alpha)
			moveScore = -search(board, -beta, -alpha, depth - 1, true, true);
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
		} else if (moveScore == -Value::VALUE_MAX + searchedPly) // Losing
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
		}

		// Alpha-Beta Pruning
		if (bestScore >= beta)
		{
			if (!(move.flags() & Move::Flag::CAPTURE))
			{
				_searchKillers[1][board.ply] = _searchKillers[0][board.ply];
				_searchKillers[0][board.ply] = move;
			}

			_transpTable.insert({ board.zKey, bestMove, byte(depth), SearchEntry::Flag::BETA });
			return beta;
		}
		
		if (bestScore > alpha)
		{
			alpha = bestScore;
			_searchHistory[move.from()][move.to()] += depth;
		}
	}

	if (legalCount == 0)
		return board.isInCheck(board.colorToMove) ? -Value::VALUE_MAX + board.ply : 0;
	
	Stats::incrementNodesSearched(legalCount);

	// Avoid putting an empty move in the Transposition Table if alpha was not raised
	if (!bestMove.empty())
	{
		// Store the result in the transposition table
		auto flag = SearchEntry::Flag::EXACT;
		if (alpha <= originalAlpha)
			flag = SearchEntry::Flag::ALPHA;
	
		bestMove.setScore(alpha);
		_transpTable.insert({ board.zKey, bestMove, byte(depth), flag });
	}
	
	return alpha;
}

int Search::searchCaptures(Board &board, int alpha, const int beta)
{
	const int standPat = evaluate(board);
	
	if (standPat >= beta)
		return standPat;
	if (standPat > alpha)
		alpha = standPat;

	if (board.ply >= MAX_DEPTH)
		return alpha;

	// Delta Pruning
	if (board.getPhase() != Phase::ENDING) // Turn it off in the Endgame
	{
		constexpr int QUEEN_VALUE = 2538;
		constexpr int PAWN_VALUE = 128;

		int bigDelta = QUEEN_VALUE;

		const bool isPromotion =
			board.history[board.historyPly - 1].move.flags() & Move::Flag::PROMOTION;
		if (isPromotion)
			bigDelta += QUEEN_VALUE - PAWN_VALUE;

		if (standPat < alpha - bigDelta)
			return alpha;
	}

	MoveList moveList(board);
	Stats::incrementNodesGenerated(moveList.size());

	MoveOrdering::sortQMoves(moveList.begin(), moveList.end());
	size_t legalCount{};
	size_t searchedMoves{};
	
	for (const Move &move : moveList)
	{
		if (!board.makeMove(move))
			continue;
		++legalCount;

		if (const auto flags = move.flags();
			!(flags & Move::CAPTURE || flags & Move::PROMOTION))
		{
			board.undoMove();
			break; // The moves are sorted so we can break if is not a capture
		}

		const int moveScore = -searchCaptures(board, -beta, -alpha);
		board.undoMove();
		++searchedMoves;

		if (moveScore >= beta)
		{
			Stats::incrementNodesSearched(searchedMoves);
			return beta;
		}
		if (moveScore > alpha)
			alpha = moveScore;
	}

	if (legalCount == 0)
		return board.isInCheck(board.colorToMove) ? -Value::VALUE_MAX + board.ply : 0;

	Stats::incrementNodesSearched(searchedMoves);

	return alpha;
}

inline int Search::evaluate(const Board &board)
{
	const int value = Evaluation::evaluate(board);
	return board.colorToMove ? value : -value;
}
