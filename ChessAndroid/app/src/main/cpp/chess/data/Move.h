#pragma once

#include <memory>

#include "Pos.h"
#include "Board.h"

class Move
{
public:
	Pos start;
	Pos dest;
	std::shared_ptr<Board> board;
	float value;

	Move() = default;

	Move(Pos start, Pos dest, Board *board)
		: start(std::move(start)), dest(std::move(dest)), board(board), value(board->evaluate()) {}

	Move(Pos start, Pos dest, Board *board, int value)
			: start(std::move(start)), dest(std::move(dest)), board(board), value(value) {}

	bool operator<(const Move &other) const
	{
		return value < other.value;
	}
};
