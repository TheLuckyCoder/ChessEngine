#include "Search.h"

#include <cassert>
#include <iostream>

#include "../Stats.h"
#include "../data/Board.h"
#include "MoveGen.h"
#include "MoveOrdering.h"
#include "Evaluation.h"
#include "../data/Psqt.h"

std::array<std::array<unsigned int, MAX_DEPTH>, 2> Search::_searchKillers{};
std::array<std::array<byte, 64>, 64> Search::_searchHistory{};
ThreadPool Search::_threadPool(1);
TranspositionTable Search::_transpTable(2);
bool Search::_quiescenceSearchEnabled{};

Move Search::findBestMove(Board board, const Settings &settings)
{
	// Apply Settings
	int depth = settings.getBaseSearchDepth();
	//	const auto threadCount = settings.getThreadCount();
	_quiescenceSearchEnabled = settings.performQuiescenceSearch();

	// If the Transposition Table wasn't resized, increment its age
	if (!_transpTable.setSize(settings.getCacheTableSizeMb()))
		_transpTable.incrementAge();

	// Update ThreadPool size if needed
	//	if (_threadPool.getThreadCount() != threadCount)
	//		_threadPool.updateThreadCount(threadCount);

	board.ply = 0;

	_searchKillers.fill({});
	_searchHistory.fill({});

	if (board.getPhase() < Phase::MIDDLE_GAME_PHASE / 3)
		++depth;

	return iterativeDeepening(board, depth);
}

Move Search::iterativeDeepening(Board &board, const int depth)
{
	std::array<Move, MAX_DEPTH> pvArray{};

	const auto getPvLine = [&board, &pvArray](const int depth) -> int
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
	int alpha = Value::VALUE_MIN;
	int beta = Value::VALUE_MAX;
	//bool fullWindow = true;

	for (int currentDepth = 1; currentDepth <= depth; ++currentDepth)
	{
		int bestScore = search(board, alpha, beta, currentDepth, true, true);
		/*if (!fullWindow && (bestScore <= alpha || bestScore >= beta)) // Eval outside window
		{
			// We need to do a full search
			alpha = Value::VALUE_MIN;
			beta = Value::VALUE_MAX;
			fullWindow = true;
			--currentDepth;
			continue;
		}*/

		const int pvMoves = getPvLine(currentDepth);
		bestMove = pvArray[0];

		std::cout << "Depth: " << currentDepth << ", Score: " << bestScore << ", PvMoves: ";
		for (int pvNum = 0; pvNum < pvMoves; ++pvNum)
			std::cout << pvArray[pvNum].toString() << ", ";

		std::cout << '\n';

		/*alpha = bestScore - 200;
		beta = bestScore + 200;
		fullWindow = false;*/
	}

	return bestMove;
}

int Search::search(Board &board, int alpha, int beta, const int depth, const bool doNull,
                   const bool doLateMovePruning)
{
	if (board.ply && (board.fiftyMoveRule == 100 || board.isRepetition()))
		return 0;

	if (board.ply == MAX_DEPTH)
		return Evaluation::evaluate(board).getInvertedValue();

	// If in check and not already extended, allow the search to be extended to depth -1
	if (depth <= 0)
		return _quiescenceSearchEnabled
			       ? searchCaptures(board, alpha, beta, depth)
			       : Evaluation::evaluate(board).getInvertedValue();

	const int originalAlpha = alpha;
	if (const SearchEntry entry = _transpTable[board.zKey];
		!entry.qSearch
		&& entry.age == _transpTable.currentAge()
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
	if (doNull && board.ply && depth >= 4 &&
		board.pieceCount[Piece{ QUEEN, color }] + board.pieceCount[Piece{ ROOK, color }] > 0 &&
		!board.isSideInCheck())
	{
		board.makeNullMove();
		const int nullScore = -search(board, -beta, -beta + 1, depth - 4, false, false);
		board.undoNullMove();

		if (nullScore >= beta)
		{
			Stats::incNullCuts();
			return beta;
		}
	}

	MoveList moveList(board);
	MoveOrdering::sortMoves(board, moveList);
	Stats::incNodesGenerated(moveList.size());

	size_t legalCount{};
	int bestScore = Value::VALUE_MIN;
	Move bestMove;

	const auto evalResult = Evaluation::evaluate(board);
	const bool isInCheck = evalResult.isInCheck;
	const int evalScore = evalResult.getInvertedValue();

	while (true)
	{
		assert(currentPly == board.ply);

		if (moveList.empty())
			break;
		const Move move = MoveOrdering::getNextMove(moveList);

		if (depth < 4
			&& board.ply > 2
			&& alpha != VALUE_MIN
			&& legalCount > 3
			&& !(move.flags() & Move::CAPTURE)
			&& !(move.flags() & Move::PROMOTION)
			&& !(move.flags() & Move::DOUBLE_PAWN_PUSH)
			&& !(move.piece() == PAWN && (col(move.to()) == 7 || col(move.to()) == 2))
			&& !isInCheck)
		{
			const int futilityEval = evalScore + futilityPruningBonus(depth);
			if (futilityEval <= alpha)
			{
				Stats::incFutilityCuts();
				if (futilityEval > bestScore)
				{
					bestScore = evalScore;
					bestMove = move;
				}
				continue;
			}
		}

		if (!board.makeMove(move))
			continue;
		++legalCount;

		int moveScore = alpha;
		bool didLmr = false;

		// Late Move Reductions
		if (doNull
			&& doLateMovePruning
			&& legalCount >= 4
			&& depth >= 3
			&& board.ply > 4
			&& !(move.flags() & Move::Flag::CAPTURE)
			&& !(move.flags() & Move::Flag::PROMOTION)
			&& !isInCheck
			&& !board.isSideInCheck())
		{
			moveScore = -search(board, -alpha - 1, -alpha, depth - 2, false, false);
			didLmr = true;
		}

		if (!didLmr || moveScore > alpha) // Search with a full window if LMR failed high
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
				_searchKillers[0][board.ply] = move.getContents();
			}

			_transpTable.insert({ board.zKey, bestMove, static_cast<short>(depth), SearchEntry::Flag::BETA });
			Stats::incBetaCuts();
			return beta;
		}

		if (bestScore > alpha)
		{
			alpha = bestScore;
			_searchHistory[move.from()][move.to()] += depth;
		}
	}

	if (legalCount == 0)
		return board.isSideInCheck() ? -Value::VALUE_MAX + board.ply : 0;

	Stats::incNodesSearched(legalCount);

	if (!bestMove.empty())
		storeTtAlphaExact(bestMove, board.zKey, alpha, originalAlpha, depth, false);

	return alpha;
}

int Search::searchCaptures(Board &board, int alpha, int beta, const int depth)
{
	const short currentPly = board.ply;
	const bool inCheck = board.isSideInCheck();

	const int originalAlpha = alpha;
	int standPat = VALUE_MIN;
	int bestScore = VALUE_MIN;
	Move bestMove;

	if (!inCheck)
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
		
		standPat = Evaluation::evaluate(board).getInvertedValue();

		if (standPat >= beta)
			return standPat;
		if (standPat > alpha)
			alpha = standPat;

		if (board.ply >= MAX_DEPTH)
			return alpha;

		// Delta Pruning
		if (board.getPhase() <= Phase::MIDDLE_GAME_PHASE / 4) // Turn it off near the Endgame
		{
			constexpr int QUEEN_VALUE = Evaluation::getPieceValue(QUEEN);
			constexpr int PAWN_VALUE = Evaluation::getPieceValue(PAWN);

			int bigDelta = QUEEN_VALUE;

			const bool isPromotion =
				board.history[board.historyPly - 1].getMove().flags() & Move::Flag::PROMOTION;
			if (isPromotion)
				bigDelta += QUEEN_VALUE - PAWN_VALUE;

			if (standPat < alpha - bigDelta)
				return alpha;
		}
	}

	MoveList moveList(board);
	Stats::incNodesGenerated(moveList.size());

	MoveOrdering::sortQMoves(moveList);
	size_t legalCount{};
	size_t searchedMoves{};

	while(true)
	{
		assert(currentPly == board.ply);

		if (moveList.empty())
			break;
		const Move move = MoveOrdering::getNextMove(moveList);

		if (!board.makeMove(move))
			continue;
		++legalCount;

		if (const auto flags = move.flags();
			!(flags & Move::CAPTURE || flags & Move::PROMOTION))
		{
			board.undoMove();
			break; // The moves are sorted so we can break if is not a capture
		}

		// Futility Pruning
		if (const int futilityEval = standPat
				+ Psqt::BONUS[move.to()][move.piece()].eg
				+ Evaluation::getPieceValue(move.promotedPiece())
				+ futilityPruningBonus(depth);
			!inCheck
			&&board.ply > 2
			&& standPat != VALUE_MIN
			&& alpha != VALUE_MIN
			&& futilityEval <= alpha
			&& !board.isSideInCheck())
		{
			board.undoMove();
			Stats::incFutilityCuts();
			continue;
		}

		const int moveScore = -searchCaptures(board, -beta, -alpha, depth - 1);
		board.undoMove();
		++searchedMoves;

		if (moveScore > bestScore)
		{
			bestScore = moveScore;
			bestMove = move;
			
			if (moveScore > alpha)
				alpha = moveScore;
		}

		if (moveScore >= beta)
		{
			Stats::incNodesSearched(searchedMoves);
			if (!inCheck)
				_transpTable.insert({ board.zKey, bestMove, static_cast<short>(depth), SearchEntry::Flag::BETA, true });
			Stats::incBetaCuts();
			return beta;
		}
	}

	if (legalCount == 0)
		return board.isSideInCheck() ? -Value::VALUE_MAX + board.ply : 0;
	
	if (!inCheck && !bestMove.empty())
		storeTtAlphaExact(bestMove, board.zKey, alpha, originalAlpha, depth, true);

	Stats::incNodesSearched(searchedMoves);

	return alpha;
}

int Search::futilityPruningBonus(const int depth)
{
	constexpr std::array BONUS{ 100, 160, 300, 500 };
	if (depth < 0)
		return BONUS[0];
	assert(depth < 4);
	return depth;
}

inline void Search::storeTtAlphaExact(Move bestMove, const U64 key, const int alpha, const int originalAlpha, const int depth, const bool qSearch)
{
	// Avoid putting an empty move in the Transposition Table if alpha was not raised
	assert(!bestMove.empty());

	// Store the result in the transposition table
	auto flag = SearchEntry::Flag::EXACT;
	if (alpha <= originalAlpha)
		flag = SearchEntry::Flag::ALPHA;

	bestMove.setScore(alpha);
	_transpTable.insert({ key, bestMove, static_cast<short>(depth), flag, qSearch });
}
