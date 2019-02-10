#pragma once

#include "../data/Pos.h"

class Board;

class MiniMax final
{
public:
	MiniMax() = delete;

	static PosPair maxMove(const Board &board);
	static PosPair minMove(const Board &board);

private:
	inline static int maxMove(const Board &board, short depth, int alpha, int beta, bool extended);
	inline static int minMove(const Board &board, short depth, int alpha, int beta, bool extended);

	static int quiescence(const Board &board, short depth, int alpha, int beta, bool isMaxPlayer);
};
