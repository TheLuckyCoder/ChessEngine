#include "NegaMax.h"

#include <algorithm>

#include "../Stats.h"
#include "../Settings.h"
#include "../data/Enums.h"
#include "../data/Board.h"
#include "../threads/NegaMaxThreadPool.h"

PosPair NegaMax::negaMax(const Board &board, const bool isWhite, const Settings &settings)
{
	auto validMoves = board.listValidMoves<Move>(isWhite);

	for (const auto &move : validMoves)
		if (move.board.state == State::WINNER_WHITE || move.board.state == State::WINNER_BLACK)
			return PosPair(move.start, move.dest);

	auto depth = settings.getBaseSearchDepth();
	const auto threadCount = settings.getThreadCount();
	if (validMoves.size() <= 15)
		++depth;
	--depth;

	NegaMaxThreadPool::createThreadPool(threadCount);

	Move bestMove = validMoves.front();
	int alpha = VALUE_MIN;

	while (!validMoves.empty())
		processWork(validMoves, bestMove, alpha, std::min<unsigned>(threadCount, validMoves.size()), depth, isWhite);

	return PosPair(bestMove.start, bestMove.dest);
}

void NegaMax::processWork(StackVector<Move, 150> &validMoves, Move &bestMove, int &alpha, const unsigned jobCount, const short depth, const bool isWhite)
{
	std::vector<ThreadPool::TaskFuture<int>> futures;
	futures.reserve(jobCount);

	for (int offset = 0; futures.size() != validMoves.size() && offset < static_cast<int>(jobCount); ++offset)
		futures.emplace_back(NegaMaxThreadPool::submitJob<int>(
			negaMaxRecursive, validMoves[offset].board, depth, VALUE_MIN, -alpha, !isWhite, false));

	for (auto i = 0u; i < futures.size(); ++i)
	{
		const int moveValue = -futures[i].get();

		if (moveValue > alpha)
		{
			bestMove = validMoves[i];
			alpha = moveValue;
		}
	}

	validMoves.erase(validMoves.begin(), validMoves.begin() + futures.size());
}

int NegaMax::negaMaxRecursive(const Board &board, short depth, int alpha, const int beta, const bool isWhite, bool extended)
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

	const auto validMoves = board.listValidMoves<Board>(isWhite);
	int bestValue = VALUE_MIN;

	if (Stats::enabled())
		++Stats::nodesSearched;

	for (const auto &move : validMoves)
	{
		if (move.score == VALUE_WINNER_WHITE || move.score == VALUE_WINNER_BLACK)
			return move.score;
		const int moveValue = -negaMaxRecursive(move, depth - 1u, -beta, -alpha, !isWhite, extended);

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
			return move.score;
		const int moveValue = -quiescence(move, depth - 1u, -beta, -alpha, !isWhite);

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
