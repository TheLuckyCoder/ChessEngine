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

	if (validMoves.size() <= 15)
		++depth;
	if (board.getPhase() == Phase::ENDING)
		++depth;

	NegaMaxThreadPool::createThreadPool(threadCount);

	Move bestMove = processWork(validMoves, std::min<unsigned>(threadCount, validMoves.size()), depth, isWhite);

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

Move NegaMax::processWork(StackVector<Move, 150> validMoves, const unsigned jobCount, const short depth, const bool isWhite)
{
	struct Future
	{
		ThreadPool::TaskFuture<int> task;
		Move move;

		Future(ThreadPool::TaskFuture<int> &&task, Move move)
			: task(std::move(task)), move(std::move(move)) {}
	};

	int alpha = VALUE_MIN;
	Move bestMove = validMoves.front();
	std::vector<Future> futures;
	futures.reserve(jobCount);

	const auto addWork = [&](const Move &move) {
		futures.emplace_back(NegaMaxThreadPool::submitJob<int>(negaMax, move.board, depth, VALUE_MIN, -alpha, !isWhite, false), move);
	};

	while (futures.size() != jobCount)
	{
		addWork(validMoves.front());
		validMoves.pop_front();
	}

	while (!futures.empty())
	{
		auto iter = futures.begin();
		while (iter != futures.end())
		{
			auto &future = *iter;
			if (future.task.ready(10))
			{
				const int moveScore = -future.task.get();
				if (moveScore > alpha)
				{
					bestMove = future.move;
					alpha = moveScore;
				}

				// Remove the Future
				futures.erase(iter);

				// Add more work if possible
				if (!validMoves.empty())
				{
					addWork(validMoves.front());
					validMoves.pop_front();
				}
			}
			else
				++iter;
		}
	}

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

	auto cache = BoardManager::searchCache.get(board.key);
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
	cache.depth = static_cast<byte>(depth);
	cache.score = board.score;
	cache.bestMove = bestScore;
	BoardManager::searchCache.insert(cache);

	return bestScore;
}

int NegaMax::quiescence(const Board &board, const short depth, int alpha, const int beta, const bool isWhite)
{
	if (depth == 0)
		return isWhite ? board.score : -board.score;

	const auto validMoves = board.listValidCaptures(isWhite);
	if (validMoves.empty())
		return isWhite ? board.score : -board.score;
	int bestValue = VALUE_MIN;

	if (Stats::enabled())
		++Stats::nodesSearched;

	for (const auto &move : validMoves)
	{
		if (move.score == VALUE_WINNER_WHITE || move.score == VALUE_WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const int moveScore = -quiescence(move, depth - 1, -beta, -alpha, !isWhite);

		if (moveScore > bestValue)
		{
			bestValue = moveScore;
			if (moveScore > alpha)
				alpha = moveScore;
		}

		if (alpha >= beta)
			break;
	}

	return bestValue;
}
