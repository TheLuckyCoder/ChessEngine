#pragma once

#include "../data/Pos.h"

class Board;

class NegaMax final
{
public:
	NegaMax() = delete;

	static PosPair negaMax(const Board &board, bool isWhite);

private:
	static int sideToMoveScore(int score, bool isWhite);
	inline static int negaMax(const Board &board, short depth, int alpha, int beta, bool isWhite, bool extended);
};
