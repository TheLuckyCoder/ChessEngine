#pragma once

#include "../data/Pos.h"

class Board;

class NegaMax final
{
public:
	NegaMax() = delete;

	static PosPair negaMax(const Board &board, bool isWhite);

private:
	inline static int negaMax(const Board &board, byte depth, int alpha, int beta, bool isWhite, bool extended);
	inline static int quiescence(const Board &board, byte depth, int alpha, int beta, bool isWhite);
};
