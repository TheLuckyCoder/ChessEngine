#include "MiniMax.h"

#include "../Move.h"
#include "../../threads/ThreadPool.hpp"

PosPair MiniMax::MaxMove(Board board, short depth)
{
	const auto moves = board.listAllMoves(true);
	depth--;

	std::vector<ThreadPool::TaskFuture<Move>> futures;
	futures.reserve(moves.size());

	for (const Move &move : moves)
	{
		futures.emplace_back(DefaultThreadPool::submitJob<Move>([](const Move &move, const short depth) {
			return MaxMove(move, depth, VALUE_MIN, VALUE_MAX);
		}, move, depth));
	}

	Move bestMove = moves.back();
	int bestMovePoints = futures.back().get().value;

	for (std::size_t i = moves.size() - 2; i > 0; --i) {
		const Move &it = moves[i];
		Move move = depth > 0 ? MinMove(it, depth, VALUE_MIN, VALUE_MAX) : it;

		if (move.value > bestMovePoints)
		{
			bestMove = it;
			bestMovePoints = move.value;
		}
	}

	return std::pair(bestMove.start, bestMove.dest);
}

PosPair MiniMax::MinMove(Board board, short depth)
{
	const auto moves = board.listAllMoves(false);
	depth--;

	std::vector<ThreadPool::TaskFuture<Move>> futures;
	futures.reserve(moves.size());

	for (const Move &move : moves)
	{
		futures.emplace_back(DefaultThreadPool::submitJob<Move>([](const Move &move, const short depth) {
			return MaxMove(move, depth, VALUE_MIN, VALUE_MAX);
		}, move, depth));
	}

	Move bestMove = moves.front();
	int bestMovePoints = futures.front().get().value;

	for (std::size_t i = 1; i < moves.size(); ++i)
	{
		const Move &it = moves[i];
		Move move = futures[i].get();

		if (move.value < bestMovePoints)
		{
			bestMove = it;
			bestMovePoints = move.value;
		}
	}

	return std::pair(bestMove.start, bestMove.dest);
}

Move MiniMax::MaxMove(const Move &parentMove, short depth, int alpha, int beta)
{
	const auto moves = parentMove.board->listAllMoves(true);

	Move bestMove = moves.back();
	int bestMovePoints = (--depth > 0 ? MinMove(bestMove, depth, alpha, beta) : bestMove).value;

	for (std::size_t i = moves.size() - 2; i > 0; --i) {
		const Move &it = moves[i];
		Move move = depth > 0 ? MinMove(it, depth, alpha, beta) : it;

		if (move.value > bestMovePoints)
		{
			bestMove = it;
			bestMovePoints = move.value;
			alpha = move.value;
		}

		if (beta <= alpha)
			break;
	}

	return bestMove;
}

Move MiniMax::MinMove(const Move &parentMove, short depth, int alpha, int beta)
{
	const auto moves = parentMove.board->listAllMoves(false);

	Move bestMove = moves.front();
	int bestMovePoints = (--depth > 0 ? MaxMove(bestMove, depth, alpha, beta) : bestMove).value;

	for (std::size_t i = 1; i < moves.size(); ++i) {
		const Move &it = moves[i];
		Move move = depth > 0 ? MaxMove(it, depth, alpha, beta) : it;

		if (move.value < bestMovePoints)
		{
			bestMove = it;
			bestMovePoints = move.value;
			beta = move.value;
		}

		if (beta <= alpha)
			break;
	}

	return bestMove;
}
