#include "NegaMax.h"

#include <algorithm>
#include <mutex>

#include "../Stats.h"
#include "../data/Board.h"
#include "../threads/NegaMaxThreadPool.h"

bool NegaMax::quiescenceSearchEnabled{};
TranspositionTable NegaMax::searchCache(1);
short NegaMax::bestMoveFound{};

RootMove NegaMax::getBestMove(const Board &board, const bool isWhite, const Settings &settings)
{
	const auto validMoves = board.listValidMoves<RootMove>(isWhite);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_WHITE || move.board.state == State::WINNER_BLACK)
			return move;

	// Apply Settings
	short depth = settings.getBaseSearchDepth() - 1;
	const auto threadCount = settings.getThreadCount();
	quiescenceSearchEnabled = settings.performQuiescenceSearch();

	// If the Transposition Table wasn't resized, clean it
	if (!searchCache.setSize(settings.getCacheTableSizeMb()))
		searchCache.clear();
	NegaMaxThreadPool::updateThreadCount(threadCount);

	if (board.getPhase() == Phase::ENDING)
		++depth;

	return negaMaxRoot(validMoves, std::min<unsigned>(threadCount, validMoves.size()), depth, !isWhite);
}

RootMove NegaMax::negaMaxRoot(StackVector<RootMove, 150> validMoves, const unsigned jobCount, const short ply, const bool isWhite)
{
	std::vector<ThreadPool::TaskFuture<void>> futures;
	futures.reserve(jobCount);

	std::mutex mutex;
	RootMove bestMove = validMoves.front();
	short alpha = VALUE_MIN;

	const auto doWork = [&] {
		mutex.lock();

		while (!validMoves.empty())
		{
			// Make a copy of the needed variables while locked
			const short beta = -alpha;
			const RootMove move = validMoves.front();
			validMoves.pop_front();

			mutex.unlock(); // Process the result asynchronously
			const short result = -negaMax(move.board, ply, VALUE_MIN, beta, isWhite, 1, false);
			mutex.lock();

			if (result > alpha)
			{
				alpha = result;
				bestMove = move;
			}
		}

		mutex.unlock();
	};

	for (auto i = 0u; i < jobCount; ++i)
		futures.emplace_back(NegaMaxThreadPool::submitJob<void>(doWork));

	for (auto &future : futures)
		future.get(); // Wait for the Search to finish

	bestMoveFound = alpha;

	return bestMove;
}

short NegaMax::negaMax(const Board &board, const short ply, short alpha, short beta, const bool isWhite, const short depth, const bool moveCountPruning)
{
	if (board.state == State::DRAW)
		return 0;
	if (ply <= 0)
	{
		if (ply == -1 || board.state != State::WHITE_IN_CHESS || board.state != State::BLACK_IN_CHESS)
			// Switch to Quiescence Search if enabled
			return quiescenceSearchEnabled ? quiescence(board, alpha, beta, isWhite) : sideToMove(board, isWhite);

		// Else, allow the search to be extended to ply -1 if in check
	}

	const short originalAlpha = alpha;
	if (const SearchCache cache = searchCache[board.key];
		board.key == cache.key && board.score == cache.boardScore && cache.ply == ply)
	{
		if (cache.flag == Flag::EXACT)
			return cache.value;
		if (cache.flag == Flag::ALPHA)
			alpha = std::max(alpha, cache.value);
		else if (cache.flag == Flag::BETA)
			beta = std::min(beta, cache.value);

		if (alpha >= beta)
			return cache.value;
	}

	if (Stats::enabled())
		++Stats::nodesSearched;

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	short bestScore = VALUE_MIN;
	short movesCount = 0;

	for (const Board &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
		{
			// Mate Pruning
			const short mateValue = VALUE_WINNER_WHITE - depth;
			if (mateValue < beta)
			{
				beta = mateValue;
				if (alpha >= mateValue) return mateValue;
			}

			if (mateValue > alpha)
			{
				alpha = mateValue;
				if (beta <= mateValue) return mateValue;
			}

			if (bestScore > mateValue)
				bestScore = mateValue;
			continue;
		}

		short moveScore = alpha + 1;

		// Late Move Reductions
		if (!moveCountPruning &&
			movesCount > 4 &&
			depth >= 3 &&
			ply >= 3 &&
			move.state != State::WHITE_IN_CHESS &&
			move.state != State::BLACK_IN_CHESS &&
			!move.isCapture &&
			!move.isPromotion)
			moveScore = -negaMax(move, ply - 2, -moveScore, -alpha, !isWhite, depth + 1, true);

		if (moveScore > alpha)
			moveScore = -negaMax(move, ply - 1, -beta, -alpha, !isWhite, depth + 1, false);

		if (moveScore > bestScore)
			bestScore = moveScore;

		// Alpha-Beta Pruning
		if (bestScore > alpha)
			alpha = bestScore;
		if (bestScore >= beta)
			break;

		++movesCount;
	}

	// Store the result in the transposition table
	Flag flag = Flag::EXACT;
	if (bestScore < originalAlpha)
		flag = Flag::ALPHA;
	else if (bestScore > beta)
		flag = Flag::BETA;

	searchCache.insert({ board.key, board.score, bestScore, ply, flag });

	return alpha;
}

short NegaMax::quiescence(const Board &board, short alpha, const short beta, const bool isWhite)
{
	if (board.state == State::DRAW)
		return 0;

	const short standPat = sideToMove(board, isWhite);

	if (standPat >= beta)
		return standPat;
	if (standPat > alpha)
		alpha = standPat;

	// Delta Pruning
	if (board.getPhase() != Phase::ENDING) // Turn it off in the Endgame
	{
		constexpr short QUEEN_VALUE = 2529;
		constexpr short PAWN_VALUE = 136;

		short bigDelta = QUEEN_VALUE;
		if (board.isPromotion)
			bigDelta += QUEEN_VALUE - PAWN_VALUE;

		if (standPat < alpha - bigDelta)
			return alpha;
	}

	const auto validMoves = board.listQuiescenceMoves(isWhite);

	if (Stats::enabled())
		++Stats::nodesSearched;

	for (const Board &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const short moveScore = -quiescence(move, -beta, -alpha, !isWhite);

		if (moveScore >= beta)
			return moveScore;
		if (moveScore > alpha)
			alpha = moveScore;
	}

	return alpha;
}

short NegaMax::negaScout(const Board &board, short ply, short alpha, short beta, bool isWhite, short depth)
{
	if (board.state == State::DRAW)
		return 0;
	if (ply == 0)
	{
		if (quiescenceSearchEnabled)
			return quiescence(board, alpha, beta, isWhite);
		return sideToMove(board, isWhite);
	}

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	short bestScore = VALUE_MIN;
	short n = beta;

	for (const auto &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const short moveScore = -negaScout(move, ply - 1, -n, -alpha, !isWhite, depth + 1);

		if (moveScore > bestScore)
			bestScore = (n == beta || ply <= 2) ? moveScore : -negaScout(move, ply - 1, -beta, -bestScore, !isWhite, depth + 1);

		if (bestScore > alpha)
			alpha = bestScore;

		if (alpha >= beta)
			break; // return alpha;

		n = alpha + 1;
	}

	return bestScore;
}

inline short NegaMax::sideToMove(const Board &board, const bool isWhite)
{
	const short value = Evaluation::evaluate(board);
	return isWhite ? value : -value;
}
