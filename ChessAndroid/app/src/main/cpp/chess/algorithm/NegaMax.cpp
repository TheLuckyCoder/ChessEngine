#include "NegaMax.h"

#include <algorithm>
#include <utility>

#include "../Stats.h"
#include "../BoardManager.h"
#include "../data/Enums.h"
#include "../data/Board.h"
#include "../threads/NegaMaxThreadPool.h"

PosPair NegaMax::getBestMove(const Board &board, const bool isWhite, const Settings &settings, const bool firstMove)
{
	const auto validMoves = board.listValidMoves<Move>(isWhite);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_WHITE || move.board.state == State::WINNER_BLACK)
			return PosPair(move.start, move.dest);

	auto depth = settings.getBaseSearchDepth() - 1;
	const auto threadCount = settings.getThreadCount();

	NegaMaxThreadPool::createThreadPool(threadCount);

	if (validMoves.size() <= 15)
		depth += 2;
	if (board.getPhase() == Phase::ENDING)
		++depth;

	const Move bestMove = negaMaxRoot(validMoves, std::min<unsigned>(threadCount, validMoves.size()), depth, !isWhite);

	searchCache.clear();

	/*if (firstMove)
	{
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(1, std::min(4, static_cast<int>(bestMoves.size())));

        int i = dist(mt);
        const auto &best = bestMoves[bestMoves.size() - i];
        return PosPair(best.start, best.dest);
	}
	return PosPair(bestMoves.back().start, bestMoves.back().dest);*/
    return PosPair(bestMove.start, bestMove.dest);
}

Move NegaMax::negaMaxRoot(StackVector<Move, 150> validMoves, const unsigned jobCount, const short depth, const bool isWhite)
{
	std::vector<ThreadPool::TaskFuture<void>> futures;
	futures.reserve(jobCount);

	std::mutex mutex;
	Move bestMove = validMoves.front();
	int alpha = VALUE_MIN;

	const auto doWork = [&] {
		mutex.lock();
		while (!validMoves.empty())
		{
			const int bestScore = alpha;
			const auto move = validMoves.front();
			validMoves.pop_front();
			mutex.unlock();

			const int result = -negaMax(move.board, depth, VALUE_MIN, -bestScore, isWhite, false);
			mutex.lock();
			if (result > alpha)
			{
				alpha = result;
				bestMove = move;
			}
		}
		mutex.unlock();
	};

	for (auto i = 0u; i != jobCount; i++)
		futures.emplace_back(NegaMaxThreadPool::submitJob<void>(doWork));

	for (auto &future : futures)
		future.get();

	return bestMove;
}

int NegaMax::negaMax(const Board &board, short depth, int alpha, const int beta, const bool isWhite, bool extended)
{
	if (depth == 0)
	{
		if (!extended && (board.state == State::WHITE_IN_CHESS || board.state == State::BLACK_IN_CHESS))
		{
			depth++;
			extended = true;
		}
		else
			return isWhite ? board.score : -board.score;
			// Switch to Quiescence Search
			//return quiescence(board, 1, alpha, beta, isWhite);
	}

	auto cache = searchCache.get(board.key);
	if (board.key == cache.key && board.score == cache.score && cache.depth == depth)
	    return cache.bestMove;

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	int bestScore = VALUE_MIN;

	if (Stats::enabled())
		++Stats::nodesSearched;

	for (const auto &move : validMoves)
	{
		if (move.score == VALUE_WINNER_WHITE || move.score == VALUE_WINNER_BLACK)
			return VALUE_WINNER_WHITE;

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

	cache.key = board.key;
	cache.depth = depth;
	cache.score = board.score;
	cache.bestMove = bestScore;
	searchCache.insert(cache);

	return bestScore;
}

int NegaMax::quiescence(const Board &board, const short depth, int alpha, const int beta, const bool isWhite)
{
	if (depth == 0)
		return isWhite ? board.score : -board.score;

	const auto validMoves = board.listValidCaptures(isWhite);
	if (validMoves.empty())
		return isWhite ? board.score : -board.score;
	int bestScore = VALUE_MIN;

	if (Stats::enabled())
		++Stats::nodesSearched;

	for (const auto &move : validMoves)
	{
		if (move.score == VALUE_WINNER_WHITE || move.score == VALUE_WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const int moveScore = -quiescence(move, depth - 1, -beta, -alpha, !isWhite);

		if (moveScore > bestScore)
		{
			bestScore = moveScore;
			if (moveScore > alpha)
				alpha = moveScore;
		}

		if (alpha >= beta)
			break;
	}

	return bestScore;
}
