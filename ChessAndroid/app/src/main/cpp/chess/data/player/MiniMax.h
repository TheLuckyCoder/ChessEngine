#pragma once

#include <utility>

#include "../Pos.h"
#include "../Board.h"
class Move;

class MiniMax
{
public:
	constexpr static int VALUE_MAX = 100000000;
	constexpr static int VALUE_MIN = -100000000;

	static PosPair MaxMove(const Board &board, short depth);
	static PosPair MinMove(const Board &board, short depth);

private:
	static Move MaxMove(const Move &parentMove, short depth, int alpha, int beta);
	static Move MinMove(const Move &parentMove, short depth, int alpha, int beta);
};
