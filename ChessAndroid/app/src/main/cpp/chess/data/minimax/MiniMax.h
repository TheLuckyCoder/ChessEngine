#pragma once

#include <utility>

#include "../Pos.h"
#include "../Board.h"
class Move;

namespace MiniMax
{
	constexpr int VALUE_MAX = 100000000;
	constexpr int VALUE_MIN = -100000000;

	PosPair MaxMove(const Board& board, short depth);
	PosPair MinMove(const Board& board, short depth);

	static int MaxMove(const Board &board, short depth, int alpha, int beta);
	static int MinMove(const Board &board, short depth, int alpha, int beta);
}
