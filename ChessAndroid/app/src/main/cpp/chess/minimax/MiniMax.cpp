#include "MiniMax.h"

#include "../data/Board.h"
#include "../threads/ThreadPool.hpp"

namespace MiniMax
{

	PosPair maxMove(const Board &board)
	{
		auto moves = board.listValidMoves<Move>(true);

		for (const auto &move : moves)
			if (move.board.state == State::WINNER_WHITE)
				return PosPair(move.start, move.dest);

		const short depth = moves.size() <= 15 ? 5 : 4;

		std::vector<ThreadPool::TaskFuture<int>> futures;
		futures.reserve(moves.size());

		for (const auto &move : moves)
			futures.emplace_back(DefaultThreadPool::submitJob<int>([](const Board &board, const short depth) {
				return maxMove(board, depth - 1, VALUE_MIN, VALUE_MAX, false);
			}, move.board, depth));

		Move bestMove = std::move(moves.back());
		int bestMovePoints = moves.back().board.value;

		for (auto i = moves.size() - 2; i > 0; --i) {
			const auto moveValue = futures[i].get();

			if (moveValue > bestMovePoints)
			{
				bestMove = std::move(moves[i]);
				bestMovePoints = moveValue;
			}
		}

		return PosPair(bestMove.start, bestMove.dest);
	}

	PosPair minMove(const Board &board)
	{
		auto moves = board.listValidMoves<Move>(false);

		for (const auto &move : moves)
			if (move.board.state == State::WINNER_BLACK)
				return PosPair(move.start, move.dest);

		const short depth = moves.size() <= 15 ? 5 : 4;

		std::vector<ThreadPool::TaskFuture<int>> futures;
		futures.reserve(moves.size());

		for (const auto &move : moves)
			futures.emplace_back(DefaultThreadPool::submitJob<int>([](const Board &board, const short depth) {
				return maxMove(board, depth - 1, VALUE_MIN, VALUE_MAX, false);
			}, move.board, depth));

		Move bestMove = std::move(moves.front());
		int bestMovePoints = futures.front().get();

		for (auto i = 1u; i < moves.size(); ++i)
		{
			const auto moveValue = futures[i].get();

			if (moveValue < bestMovePoints)
			{
				bestMove = std::move(moves[i]);
				bestMovePoints = moveValue;
			}
		}

		return PosPair(bestMove.start, bestMove.dest);
	}

	int maxMove(const Board &board, short depth, int alpha, const int beta, bool extended)
	{
		const auto moves = board.listValidMoves<Board>(true);
		depth--;

		if (depth == 0)
		{
			if (!extended && (board.state == State::WHITE_IN_CHESS || board.state == State::BLACK_IN_CHESS))
			{
				depth++;
				extended = true;
			}
			else if (!moves.empty())
				return moves.back().value;
		}

		int bestMovePoints = VALUE_MIN;

		for (auto it = moves.rbegin(); it != moves.rend(); ++it)
		{
			const auto &move = *it;
			if (move.value == VALUE_WINNER_WHITE)
				return VALUE_WINNER_WHITE;
			const int moveValue = minMove(move, depth, alpha, beta, extended);

			if (moveValue > bestMovePoints)
			{
				bestMovePoints = moveValue;

				if (bestMovePoints > alpha)
					alpha = bestMovePoints;
			}

			if (beta < alpha)
				break;
		}

		return bestMovePoints;
	}

	int minMove(const Board &board, short depth, const int alpha, int beta, bool extended)
	{
		const auto moves = board.listValidMoves<Board>(false);
		depth--;

		if (depth == 0)
		{
			if (!extended && (board.state == State::WHITE_IN_CHESS || board.state == State::BLACK_IN_CHESS))
			{
				depth++;
				extended = true;
			}
			else if (!moves.empty())
				return moves.front().value;
		}

		int bestMovePoints = VALUE_MAX;

		for (const auto &move : moves)
		{
			if (move.value == VALUE_WINNER_BLACK)
				return VALUE_WINNER_BLACK;
			const int moveValue = maxMove(move, depth, alpha, beta, extended);

			if (moveValue < bestMovePoints)
			{
				bestMovePoints = moveValue;

				if (bestMovePoints < beta)
					beta = bestMovePoints;
			}

			if (beta < alpha)
				break;
		}

		return bestMovePoints;
	}

}
