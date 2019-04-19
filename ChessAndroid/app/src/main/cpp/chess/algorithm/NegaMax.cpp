#include "NegaMax.h"

#include <algorithm>
#include <mutex>
#include <utility>

#include "../Stats.h"
#include "../BoardManager.h"
#include "../data/Enums.h"
#include "../data/Board.h"
#include "../threads/NegaMaxThreadPool.h"

bool NegaMax::quiescenceSearchEnabled;
TranspositionTable NegaMax::searchCache(1);

PosPair NegaMax::getBestMove(const Board &board, const bool isWhite, const Settings &settings)
{
	const auto validMoves = board.listValidMoves<RootMove>(isWhite);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_WHITE || move.board.state == State::WINNER_BLACK)
			return PosPair(move.start, move.dest);

	// Apply Settings
	auto depth = settings.getBaseSearchDepth() - 1;
	const auto threadCount = settings.getThreadCount();
	quiescenceSearchEnabled = settings.performQuiescenceSearch();

	searchCache.setSize(settings.getCacheTableSizeMb());
	NegaMaxThreadPool::updateThreadCount(threadCount);

	if (validMoves.size() <= 15)
		++depth;
	if (board.getPhase() == Phase::ENDING)
		++depth;

	const RootMove bestMove = negaMaxRoot(validMoves, std::min<unsigned>(threadCount, validMoves.size()), depth, !isWhite);

	searchCache.clear();

    return PosPair(bestMove.start, bestMove.dest);
}

RootMove NegaMax::negaMaxRoot(StackVector<RootMove, 150> validMoves, const unsigned jobCount, const short depth, const bool isWhite)
{
	std::vector<ThreadPool::TaskFuture<void>> futures;
	futures.reserve(jobCount);

	std::mutex mutex;
	RootMove bestMove = validMoves.front();
	int alpha = VALUE_MIN;

	const auto doWork = [&] {
		mutex.lock();

		while (!validMoves.empty())
		{
			// Make a copy of the needed variables while locked
			const int beta = -alpha;
			const RootMove move = validMoves.front();
			validMoves.pop_front();

			mutex.unlock(); // Process the result asynchronously
			const int result = -negaMax(move.board, depth, VALUE_MIN, beta, isWhite, false);
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

	return bestMove;
}

int NegaMax::negaMax(const Board &board, short depth, int alpha, int beta, const bool isWhite, bool extended)
{
	if (depth == 0)
	{
		if (board.state == State::DRAW)
			return 0;
		if (!extended && (board.state == State::WHITE_IN_CHESS || board.state == State::BLACK_IN_CHESS))
		{
			depth++;
			extended = true;
		}
		else if (quiescenceSearchEnabled) // Switch to Quiescence Search if enabled
			return quiescence(board, alpha, beta, isWhite);
		else
			return sideToMove(board, isWhite);
	}

	const int originalAlpha = alpha;
	if (const SearchCache cache = searchCache[board.key];
		board.key == cache.key && board.score == cache.boardScore && cache.depth >= depth)
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

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	int bestScore = VALUE_MIN;

	if (Stats::enabled())
		++Stats::nodesSearched;

	for (const auto &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
		{
			bestScore = VALUE_WINNER_WHITE;
			break;
		}

		const int moveScore = -negaMax(move, depth - 1, -beta, -alpha, !isWhite, extended);

		if (moveScore > bestScore)
		{
			bestScore = moveScore;
			if (moveScore > alpha)
				alpha = moveScore;
		}

		if (alpha >= beta)
			break;
	}

	if (!extended)
	{
		Flag flag = Flag::EXACT;
		if (bestScore < originalAlpha)
			flag = Flag::ALPHA;
		else if (bestScore > beta)
			flag = Flag::BETA;

		searchCache.insert({ board.key, board.score, bestScore, depth, flag });
	}

	return bestScore;
}

int NegaMax::quiescence(const Board &board, int alpha, const int beta, const bool isWhite)
{
	if (board.state == State::DRAW)
		return 0;

	const int standPat = sideToMove(board, isWhite);

	if (standPat >= beta)
		return standPat;
	if (standPat > alpha)
		alpha = standPat;

	const auto validMoves = board.listValidCaptures(isWhite);

	if (Stats::enabled())
		++Stats::nodesSearched;

	for (const auto &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const int moveScore = -quiescence(move, -beta, -alpha, !isWhite);

		if (moveScore >= beta)
			return moveScore;
		if (moveScore > alpha)
			alpha = moveScore;
	}

	return alpha;
}

int NegaMax::negaScout(const Board &board, short depth, int alpha, int beta, bool isWhite)
{
	if (board.state == State::DRAW)
		return 0;
	if (depth == 0)
	{
		if (quiescenceSearchEnabled)
			return quiescence(board, alpha, beta, isWhite);
		return sideToMove(board, isWhite);
	}

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	int bestScore = VALUE_MIN;
	int n = beta;

	for (const auto &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const int moveScore = -negaScout(move, depth -1, -n, -alpha, !isWhite);

		if (moveScore > bestScore)
		{
			bestScore = (n == beta || depth <= 2) ? moveScore : -negaScout(move, depth - 1, -beta, -bestScore, !isWhite);
		}

		if (bestScore > alpha)
			alpha = bestScore;

		if (alpha >= beta)
			break; // return alpha;

		n = alpha + 1;
	}

	return bestScore;
}

inline int NegaMax::sideToMove(const Board &board, const bool isWhite)
{
	return isWhite ? board.score : -board.score;
}
