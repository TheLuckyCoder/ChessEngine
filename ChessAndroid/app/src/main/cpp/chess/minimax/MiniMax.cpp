#include "MiniMax.h"

#include "../data/Enums.h"
#include "../data/Board.h"
#include "../threads/ThreadPool.hpp"

PosPair MiniMax::maxMove(const Board &board)
{
	auto validMoves = board.listValidMoves<Move>(true);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_WHITE)
			return PosPair(move.start, move.dest);

	const short depth = validMoves.size() <= 15 ? 5 : 4;

	std::vector<ThreadPool::TaskFuture<int>> futures;
	futures.reserve(validMoves.size());

	for (const auto &move : validMoves)
		futures.emplace_back(DefaultThreadPool::submitJob<int>([](const Board &board, const short depth) {
			return minMove(board, depth, VALUE_MIN, VALUE_MAX, false);
		}, move.board, depth - 1));

	Move *bestMove = &validMoves.front();
	int bestMovePoints = validMoves.back().board.value;

	for (auto i = 1u; i < validMoves.size(); ++i)
	{
		const auto moveValue = futures[i].get();

		if (moveValue < bestMovePoints)
		{
			bestMove = &validMoves[i];
			bestMovePoints = moveValue;
		}
	}

	return PosPair(bestMove->start, bestMove->dest);
}

PosPair MiniMax::minMove(const Board &board)
{
	auto validMoves = board.listValidMoves<Move>(false);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_BLACK)
			return PosPair(move.start, move.dest);

	const short depth = validMoves.size() <= 15 ? 5 : 4;

	std::vector<ThreadPool::TaskFuture<int>> futures;
	futures.reserve(validMoves.size());

	for (const auto &move : validMoves)
		futures.emplace_back(DefaultThreadPool::submitJob<int>([](const Board &board, const short depth) {
			return maxMove(board, depth, VALUE_MIN, VALUE_MAX, false);
		}, move.board, depth - 1));

	Move *bestMove = &validMoves.front();
	int bestMovePoints = futures.front().get();

	for (auto i = 1u; i < validMoves.size(); ++i)
	{
		const auto moveValue = futures[i].get();

		if (moveValue < bestMovePoints)
		{
			bestMove = &validMoves[i];
			bestMovePoints = moveValue;
		}
	}

	return PosPair(bestMove->start, bestMove->dest);
}

inline int MiniMax::maxMove(const Board &board, short depth, int alpha, const int beta, bool extended)
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
			return moves.front().value;
		else if (board.state == State::WHITE_IN_CHESS)
			return VALUE_WINNER_BLACK;
	}

	int bestMovePoints = VALUE_MIN;

	for (const auto &move : moves)
	{
		if (move.value == VALUE_WINNER_WHITE)
			return VALUE_WINNER_WHITE;
		const int moveValue = minMove(move, depth, alpha, beta, extended);

		if (moveValue > bestMovePoints)
		{
			bestMovePoints = moveValue;

			if (bestMovePoints > alpha)
				alpha = bestMovePoints;
		}

		if (beta <= alpha)
			break;
	}

	return bestMovePoints;
}

inline int MiniMax::minMove(const Board &board, short depth, const int alpha, int beta, bool extended)
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
		else if (board.state == State::WHITE_IN_CHESS)
			return VALUE_WINNER_WHITE;
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

		if (beta <= alpha)
			break;
	}

	return bestMovePoints;
}

int MiniMax::quiescence(const Board &board, short depth, int alpha, int beta, const bool isMaxPlayer)
{
	const auto moves = board.listValidMovesQ(isMaxPlayer);
	depth--;

	if (depth == 0)
	{
		if (!moves.empty())
			return moves.front().value;
	}

	int bestMovePoints = isMaxPlayer ? VALUE_MIN : VALUE_MAX;

	for (const auto &move : moves)
	{
		if (move.value == VALUE_WINNER_BLACK)
			return VALUE_WINNER_BLACK;
		const int moveValue = quiescence(move, depth, alpha, beta, !isMaxPlayer);

		if (isMaxPlayer && moveValue > bestMovePoints)
		{
			bestMovePoints = moveValue;
			if (bestMovePoints > alpha)
				alpha = bestMovePoints;
		}
		else if (!isMaxPlayer && moveValue < bestMovePoints)
		{
			bestMovePoints = moveValue;
			if (bestMovePoints < beta)
				beta = bestMovePoints;
		}

		if (beta <= alpha)
			break;
	}

	return bestMovePoints;
}
