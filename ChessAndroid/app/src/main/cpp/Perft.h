#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "chess/data/Board.h"
#include "chess/algorithm/MoveGen.h"

static U64 perft(Board &board, const unsigned depth)
{
	if (board.fiftyMoveRule == 100)
		return 0;

	if (depth == 0)
		return 1;

	MoveList moveList(board);

	size_t legalCount{};

	for (const Move move : moveList)
	{
		if (!board.makeMove(move))
			continue;

		legalCount += perft(board, depth - 1);
		board.undoMove();
	}

	return legalCount;
}

void perftTest(const std::string &fen, const std::initializer_list<U64> perftResults)
{
	Board board;
	board.setToFen(fen);

	constexpr auto TAG = "Perft: ";

	std::vector<U64> perftVector(perftResults);

	for (unsigned i = 1; i < perftVector.size(); ++i)
	{
		std::cout << TAG << "Starting Depth " << i << " Test\n";

		const auto startTime = std::chrono::high_resolution_clock::now();
		const U64 nodeCount = perft(board, i);

		const auto currentTime = std::chrono::high_resolution_clock::now();
		const double timeNeeded = std::chrono::duration<double, std::milli>(currentTime - startTime).count();

		std::cout << TAG << "Time needed: " << timeNeeded << '\n';

		const auto expectedNodeCount = perftVector[i];
		if (nodeCount != expectedNodeCount)
			std::cerr << TAG << "Nodes count: " << nodeCount << '/' << expectedNodeCount << '\n';
		else
			std::cout << TAG << "Nodes count: " << nodeCount <<'\n';
	}
}
