#include "MiniMax.h"

#include "../Move.h"

PosPair MiniMax::MaxMove(Board board, short depth)
{
	bool isBestMoveInitialized = false;
	Move bestMove;
    int bestMovePoints;

	const auto moves = board.listAllMoves(false);

	for (auto it = moves.rbegin(); it != moves.rend(); ++it) {
		Move move = MinMove(*it, --depth, VALUE_MIN, VALUE_MAX);

		if (!isBestMoveInitialized)
		{
			isBestMoveInitialized = true;
			bestMove = *it;
			bestMovePoints = move.value;
		}
		else if (move.value > bestMovePoints)
		{
			bestMove = *it;
			bestMovePoints = move.value;
		}
	}

	return std::pair(bestMove.start, bestMove.dest);
}

PosPair MiniMax::MinMove(Board board, short depth)
{
	bool isBestMoveInitialized = false;
	Move bestMove;
    int bestMovePoints;

	const auto moves = board.listAllMoves(false);

	for (const Move &it : moves) {
		Move move = MaxMove(it, --depth, VALUE_MIN, VALUE_MAX);

		if (!isBestMoveInitialized)
		{
			isBestMoveInitialized = true;
			bestMove = it;
			bestMovePoints = move.value;
		}
		else if (move.value < bestMovePoints)
		{
			bestMove = it;
			bestMovePoints = move.value;
		}
	}

	return std::pair(bestMove.start, bestMove.dest);
}

Move MiniMax::MaxMove(const Move &parentMove, short depth, int alpha, int beta)
{
	bool isBestMoveInitialized = false;
	Move bestMove;
	int bestMovePoints;

	const auto moves = parentMove.board->listAllMoves(true);

	for (auto it = moves.rbegin(); it != moves.rend(); ++it) {
		Move move = --depth > 0 ? MinMove(*it, depth, alpha, beta) : *it;

		if (!isBestMoveInitialized)
		{
			isBestMoveInitialized = true;
			bestMove = *it;
			bestMovePoints = move.value;
		}
		else if (move.value > bestMovePoints)
		{
			bestMove = *it;
			bestMovePoints = move.value;
			alpha = move.value;
		}

		if (beta < alpha)
			break;
	}

	return bestMove;
}

Move MiniMax::MinMove(const Move &parentMove, short depth, int alpha, int beta)
{
	bool isBestMoveInitialized = false;
	Move bestMove;
    int bestMovePoints;

	const auto moves = parentMove.board->listAllMoves(false);

	for (const Move &it : moves) {
		Move move = --depth > 0 ? MaxMove(it, depth, alpha, beta) : it;

		if (!isBestMoveInitialized)
		{
			isBestMoveInitialized = true;
			bestMove = it;
			bestMovePoints = move.value;
		}
		else if (move.value < bestMovePoints)
		{
			bestMove = it;
			bestMovePoints = move.value;
			beta = move.value;
		}

		if (beta < alpha)
			break;
	}

	return bestMove;
}
