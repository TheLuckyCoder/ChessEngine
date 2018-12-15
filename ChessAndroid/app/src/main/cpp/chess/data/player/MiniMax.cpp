#include "MiniMax.h"

#include "../Move.h"
#include "../../threads/ThreadPool.hpp"

PosPair MiniMax::MaxMove(const Board &board, short depth)
{
	auto moves = board.listAllMoves(true);
	depth--;
	int alpha = VALUE_MIN;

	std::vector<ThreadPool::TaskFuture<Move>> futures;
	futures.reserve(moves.size());

	for (const Move &move : moves)
		futures.emplace_back(DefaultThreadPool::submitJob<Move>([&move, &alpha](const short depth) {
			return MaxMove(move, depth, alpha, VALUE_MAX);
		}, depth));

	Move bestMove = std::move(moves.back());
	int bestMovePoints = futures.back().get().value;

	for (std::size_t i = moves.size() - 2; i > 0; --i) {
		const Move move = futures[i].get();

		if (move.value > bestMovePoints)
		{
			bestMove = std::move(moves[i]);
			bestMovePoints = alpha = move.value;
		}
	}

	return std::pair(bestMove.start, bestMove.dest);
}

PosPair MiniMax::MinMove(const Board &board, short depth)
{
	auto moves = board.listAllMoves(false);
	depth--;
	int beta = VALUE_MAX;

	std::vector<ThreadPool::TaskFuture<Move>> futures;
	futures.reserve(moves.size());

	for (const Move &move : moves)
		futures.emplace_back(DefaultThreadPool::submitJob<Move>([&move, &beta](const short depth) {
			return MaxMove(move, depth, VALUE_MIN, beta);
		}, depth));

	Move bestMove = std::move(moves.front());
	int bestMovePoints = futures.front().get().value;

	for (std::size_t i = 1; i < moves.size(); ++i)
	{
		const Move move = futures[i].get();

		if (move.value < bestMovePoints)
		{
			bestMove = std::move(moves[i]);
			bestMovePoints = beta = move.value;
		}
	}

	return std::pair(bestMove.start, bestMove.dest);
}

Move MiniMax::MaxMove(const Move &parentMove, short depth, int alpha, int beta)
{
	const auto moves = parentMove.board->listAllMoves(true);

	Move bestMove = std::move(moves.back());
	int bestMovePoints = (--depth > 0 ? MinMove(bestMove, depth, alpha, beta) : bestMove).value;

	for (std::size_t i = moves.size() - 2; i > 0; --i) {
		const Move &it = moves[i];
		const Move move = depth > 0 ? MinMove(it, depth, alpha, beta) : it;

		if (move.value > bestMovePoints)
		{
			bestMove = std::move(it);
			bestMovePoints = alpha = move.value;
		}

		if (beta <= alpha)
			break;
	}

	return bestMove;
}

Move MiniMax::MinMove(const Move &parentMove, short depth, int alpha, int beta)
{
	const auto moves = parentMove.board->listAllMoves(false);

	Move bestMove = std::move(moves.front());
	int bestMovePoints = (--depth > 0 ? MaxMove(bestMove, depth, alpha, beta) : bestMove).value;

	for (std::size_t i = 1; i < moves.size(); ++i) {
		const Move &it = moves[i];
		const Move move = depth > 0 ? MaxMove(it, depth, alpha, beta) : it;

		if (move.value < bestMovePoints)
		{
			bestMove = std::move(it);
			bestMovePoints = beta = move.value;
		}

		if (beta <= alpha)
			break;
	}

	return bestMove;
}
