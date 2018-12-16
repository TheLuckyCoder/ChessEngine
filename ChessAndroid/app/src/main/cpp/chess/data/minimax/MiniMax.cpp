#include "MiniMax.h"

#include "../Move.h"
#include "../../threads/ThreadPool.hpp"

namespace MiniMax
{

	PosPair MaxMove(const Board &board, short depth)
	{
		auto moves = board.listAllMoves(true);
		depth--;
		int alpha = VALUE_MIN;

		std::vector<ThreadPool::TaskFuture<int>> futures;
		futures.reserve(moves.size());

		for (const Move &move : moves)
			futures.emplace_back(DefaultThreadPool::submitJob<int>([&alpha](const Board &board, const short depth) {
				return MaxMove(board, depth, alpha, VALUE_MAX);
			}, move.board, depth));

		Move bestMove = std::move(moves.back());
		int bestMovePoints = futures.back().get();

		for (auto i = moves.size() - 2; i > 0; --i) {
			const auto value = futures[i].get();

			if (value > bestMovePoints)
			{
				bestMove = std::move(moves[i]);
				bestMovePoints = alpha = value;
			}
		}

		return std::pair(bestMove.start, bestMove.dest);
	}

	PosPair MinMove(const Board &board, short depth)
	{
		auto moves = board.listAllMoves(false);
		depth--;
		int beta = VALUE_MAX;

		std::vector<ThreadPool::TaskFuture<int>> futures;
		futures.reserve(moves.size());

		for (const Move &move : moves)
			futures.emplace_back(DefaultThreadPool::submitJob<int>([&beta](const Board &board, const short depth) {
				return MaxMove(board, depth, VALUE_MIN, beta);
			}, move.board, depth));

		Move bestMove = std::move(moves.front());
		int bestMovePoints = futures.front().get();

		for (std::size_t i = 1; i < moves.size(); ++i)
		{
			const auto value = futures[i].get();

			if (value < bestMovePoints)
			{
				bestMove = std::move(moves[i]);
				bestMovePoints = beta = value;
			}
		}

		return std::pair(bestMove.start, bestMove.dest);
	}

	int MaxMove(const Board &board, short depth, int alpha, const int beta)
	{
		const auto moves = board.listAllMoves(true);

		int bestMovePoints = --depth > 0 ? MinMove(moves.back().board, depth, alpha, beta) : moves.back().value;

		for (auto i = moves.size() - 2; i > 0; --i) {
			const Move &it = moves[i];
			const int value = depth > 0 ? MinMove(it.board, depth, alpha, beta) : it.value;

			if (value > bestMovePoints)
				bestMovePoints = alpha = value;

			if (beta <= alpha)
				break;
		}

		return bestMovePoints;
	}

	int MinMove(const Board &board, short depth, const int alpha, int beta)
	{
		const auto moves = board.listAllMoves(false);

		int bestMovePoints = --depth > 0 ? MaxMove(moves.front().board, depth, alpha, beta) : moves.front().value;

		for (std::size_t i = 1; i < moves.size(); ++i) {
			const Move &it = moves[i];
			const int value = depth > 0 ? MaxMove(it.board, depth, alpha, beta) : it.value;

			if (value < bestMovePoints)
				bestMovePoints = beta = value;

			if (beta <= alpha)
				break;
		}

		return bestMovePoints;
	}

}
