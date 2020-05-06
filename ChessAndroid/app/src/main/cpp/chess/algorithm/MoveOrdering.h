#pragma once

#include "MoveGen.h"

class Board;

namespace MoveOrdering
{
	constexpr int NORMAL_SCORE = 1000000;
	constexpr int PV_SCORE = NORMAL_SCORE * 2;

	void sortMoves(const Board &board, MoveList &moveList) noexcept;

	void sortQMoves(MoveList &moveList) noexcept;

	Move getNextMove(MoveList &moveList) noexcept;
}
