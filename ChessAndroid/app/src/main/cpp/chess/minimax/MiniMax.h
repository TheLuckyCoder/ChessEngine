#pragma once

#include "../data/Pos.h"

class Board;

namespace MiniMax
{
	constexpr int VALUE_MAX = 2000000000;
	constexpr int VALUE_MIN = -2000000000;
	constexpr int VALUE_WINNER_WHITE = 32767;
	constexpr int VALUE_WINNER_BLACK = -32767;

	PosPair maxMove(const Board &board);
	PosPair minMove(const Board &board);

	static int maxMove(const Board &board, short depth, int alpha, int beta, bool extended);
	static int minMove(const Board &board, short depth, int alpha, int beta, bool extended);

	static int quiescence(const Board &board, short depth, int alpha, int beta, bool isMaxPlayer);
}
