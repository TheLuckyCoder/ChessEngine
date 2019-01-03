#include "MiniMax.h"

#include "../Move.h"
#include "../../threads/ThreadPool.hpp"

namespace MiniMax
{

	PosPair MaxMove(const Board &board)
	{
		auto moves = board.listValidMoves(true);

		std::vector<ThreadPool::TaskFuture<int>> futures;
		futures.reserve(moves.size());

		for (const auto &move : moves)
			futures.emplace_back(DefaultThreadPool::submitJob<int>([](const Board &board) {
				return MaxMove(board, DEPTH - 1, VALUE_MIN, VALUE_MAX);
			}, move.board));

		Move bestMove = moves.front();
		int bestMovePoints = VALUE_MIN;

		for (auto i = moves.size() - 1; i > 0; --i) {
			const auto moveValue = futures[i].get();

			if (moveValue > bestMovePoints)
			{
				bestMove = std::move(moves[i]);
				bestMovePoints = moveValue;
			}
		}

		return std::pair(bestMove.start, bestMove.dest);
	}

	PosPair MinMove(const Board &board)
	{
		auto moves = board.listValidMoves(false);

		std::vector<ThreadPool::TaskFuture<int>> futures;
		futures.reserve(moves.size());

		for (const auto &move : moves)
			futures.emplace_back(DefaultThreadPool::submitJob<int>([](const Board &board) {
				return MaxMove(board, DEPTH - 1, VALUE_MIN, VALUE_MAX);
			}, move.board));

		Move bestMove = moves.front();
		int bestMovePoints = VALUE_MAX;

		for (std::size_t i = 0; i < moves.size(); ++i)
		{
			const auto moveValue = futures[i].get();

			if (moveValue < bestMovePoints)
			{
				bestMove = std::move(moves[i]);
				bestMovePoints = moveValue;
			}
		}

		return std::pair(bestMove.start, bestMove.dest);
	}

	int MaxMove(const Board &board, short depth, int alpha, const int beta)
	{
		const auto moves = board.listValidMoves(true);
		depth--;

		int bestMovePoints = VALUE_MIN;

		for (auto it = moves.rbegin(); it != moves.rend(); ++it) {
			const auto &move = *it;
			const int moveValue = depth > 0 ? MinMove(move.board, depth, alpha, beta) : move.value;

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

	int MinMove(const Board &board, short depth, const int alpha, int beta)
	{
		const auto moves = board.listValidMoves(false);
		depth--;

		int bestMovePoints = VALUE_MAX;

		for (const auto &move : moves) {
			const int moveValue = depth > 0 ? MaxMove(move.board, depth, alpha, beta) : move.value;

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
