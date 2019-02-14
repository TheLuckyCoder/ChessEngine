#include "NegaMax.h"

#include "../Stats.h"
#include "../Settings.h"
#include "../data/Enums.h"
#include "../data/Board.h"
#include "../threads/NegaMaxThreadPool.h"

PosPair NegaMax::negaMax(const Board &board, const bool isWhite, const Settings &settings)
{
	const auto validMoves = board.listValidMoves<Move>(isWhite);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_WHITE || move.board.state == State::WINNER_BLACK)
			return PosPair(move.start, move.dest);

	auto depth = settings.getBaseSearchDepth();
	if (validMoves.size() <= 15)
		depth++;
	NegaMaxThreadPool::createThreadPool(settings.getThreadCount());

	std::vector<ThreadPool::TaskFuture<int>> futures;
	futures.reserve(validMoves.size());

	for (const auto &move : validMoves)
		futures.emplace_back(NegaMaxThreadPool::submitJob<int>([](const Board &board, const byte depth, const bool isWhite) {
			return -negaMax(board, depth, VALUE_MIN, VALUE_MAX, !isWhite, false);
		}, move.board, --depth, isWhite));

	const Move *bestMove = &validMoves.front();
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

int NegaMax::negaMax(const Board &board, byte depth, int alpha, const int beta, const bool isWhite, bool extended)
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
			//return -quiescence(board, 1, -beta, -alpha, !isWhite);
	}

	++Stats::nodesSearched;

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	int bestValue = VALUE_MIN;

	for (const auto &move : validMoves)
	{
		if (move.value == VALUE_WINNER_WHITE || move.value == VALUE_WINNER_BLACK)
			return move.value;
		const int moveValue = -negaMax(move, depth - 1u, -beta, -alpha, !isWhite, extended);

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

int NegaMax::quiescence(const Board &board, const byte depth, int alpha, const int beta, const bool isWhite)
{
	if (depth == 0)
		return isWhite ? board.value : -board.value;

	const auto validMoves = board.listValidCaptures(isWhite);

	for (const auto &move : validMoves)
	{
		if (move.value == VALUE_WINNER_WHITE || move.value == VALUE_WINNER_BLACK)
			return move.value;
		const int moveValue = -quiescence(move, depth - 1u, -beta, -alpha, !isWhite);

		if (moveValue > alpha)
			alpha = moveValue;

		if (alpha >= beta)
			break;
	}

	return alpha;
}
