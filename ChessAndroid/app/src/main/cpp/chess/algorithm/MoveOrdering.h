#pragma once

#include "MoveGen.h"

class Board;

namespace MoveOrdering
{
	void sortMoves(const Board &board, MoveList &moveList) noexcept;

	void sortQMoves(MoveList &moveList) noexcept;
}
