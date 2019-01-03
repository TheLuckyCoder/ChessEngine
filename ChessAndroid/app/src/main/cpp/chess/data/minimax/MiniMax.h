#pragma once

#include "../Pos.h"
#include "../Board.h"

namespace MiniMax
{

	constexpr int VALUE_MAX = 100000000;
	constexpr int VALUE_MIN = -100000000;
	constexpr short DEPTH = 5;

	PosPair MaxMove(const Board &board);
	PosPair MinMove(const Board &board);

	static int MaxMove(const Board &board, short depth, int alpha, int beta);
	static int MinMove(const Board &board, short depth, int alpha, int beta);

}
