#pragma once

#include "../data/Pos.h"

class Board;

namespace MiniMax
{
	constexpr int VALUE_MAX = 2000000000;
	constexpr int VALUE_MIN = -2000000000;
	constexpr int VALUE_WINNER_WHITE = 32767;
	constexpr int VALUE_WINNER_BLACK = -32767;

	PosPair MaxMove(const Board &board);
	PosPair MinMove(const Board &board);

	static int MaxMove(const Board &board, short depth, int alpha, int beta);
	static int MinMove(const Board &board, short depth, int alpha, int beta);

}
