#pragma once

#include "../data/Pos.h"

class Board;

class MiniMax final
{
public:
	MiniMax() = delete;

	constexpr static int VALUE_MAX = 2000000000;
	constexpr static int VALUE_MIN = -2000000000;
	constexpr static int VALUE_WINNER_WHITE = 32767;
	constexpr static int VALUE_WINNER_BLACK = -32767;

	static PosPair maxMove(const Board &board);
	static PosPair minMove(const Board &board);

private:
	inline static int maxMove(const Board &board, short depth, int alpha, int beta, bool extended);
	inline static int minMove(const Board &board, short depth, int alpha, int beta, bool extended);

	static int quiescence(const Board &board, short depth, int alpha, int beta, bool isMaxPlayer);
};
