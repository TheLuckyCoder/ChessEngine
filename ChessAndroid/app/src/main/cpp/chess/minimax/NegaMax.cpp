#include "NegaMax.h"

#include "../data/Enums.h"
#include "../data/Board.h"
#include "../threads/ThreadPool.hpp"

PosPair NegaMax::negaMax(const Board &board, const bool isWhite)
{
	auto validMoves = board.listValidMoves<Move>(isWhite);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_BLACK)
			return PosPair(move.start, move.dest);

	const byte depth = validMoves.size() <= 15 ? 5 : 4;

	std::vector<ThreadPool::TaskFuture<int>> futures;
	futures.reserve(validMoves.size());

	for (const auto &move : validMoves)
		futures.emplace_back(DefaultThreadPool::submitJob<int>([](const Board &board, const byte depth, const bool isWhite) {
			return -negaMax(board, depth - 1, VALUE_MIN, VALUE_MAX, !isWhite, false);
		}, move.board, depth, isWhite));

	Move *bestMove = &validMoves.front();
	int bestMovePoints = futures.front().get();

	for (auto i = 1u; i < validMoves.size(); ++i)
	{
		const auto moveValue = futures[i].get();

		if (moveValue > bestMovePoints)
		{
			bestMove = &validMoves[i];
			bestMovePoints = moveValue;
		}
	}

	return PosPair(bestMove->start, bestMove->dest);
}

inline int NegaMax::negaMax(const Board &board, byte depth, int alpha, const int beta, const bool isWhite, bool extended)
{
	if (depth == 0)
	{
		if (!extended && (board.state == State::WHITE_IN_CHESS || board.state == State::BLACK_IN_CHESS))
		{
			depth++;
			extended = true;
		}
		else
			return isWhite ? board.value : -board.value;
	}

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	int bestValue = VALUE_MIN;

	for (const auto &move : validMoves)
	{
		if (move.value == VALUE_WINNER_WHITE || move.value == VALUE_WINNER_BLACK)
			return move.value;
		const int moveValue = -negaMax(move, depth - 1, -beta, -alpha, !isWhite, extended);

		if (moveValue > bestValue)
		{
			bestValue = moveValue;
			if (moveValue > alpha)
				alpha = moveValue;
		}

		if (alpha >= beta)
			break;
	}

	return bestValue;
}

inline int NegaMax::quiescence(const Board &board, byte const depth, int alpha, const int beta, const bool isWhite)
{
	if (depth == 0)
		return isWhite ? board.value : -board.value;

	const auto validMoves = board.listValidCaptures(isWhite);
	int bestValue = VALUE_MIN;

	for (const auto &move : validMoves)
	{
		if (move.value == VALUE_WINNER_WHITE || move.value == VALUE_WINNER_BLACK)
			return move.value;
		const int moveValue = -quiescence(move, depth - 1, -beta, -alpha, !isWhite);

		if (moveValue > bestValue)
		{
			bestValue = moveValue;
			if (moveValue > alpha)
				alpha = moveValue;
		}

		if (alpha >= beta)
			break;
	}

	return bestValue;
}