#pragma once

#include "../data/Move.h"

class Board;

namespace MoveOrdering
{
	void sortMoves(const Board &board, Move *begin, Move *end) noexcept;

	void sortQMoves(Move *begin, Move *end) noexcept;
}
