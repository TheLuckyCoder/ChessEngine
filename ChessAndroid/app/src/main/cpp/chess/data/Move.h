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
	int value;

	Move()
		: value{} {}

	Move(const Pos start, const Pos dest, Board *board)
		: start(start), dest(dest), board(board), value(board->evaluate()) {}

	Move(const Pos start, const Pos dest, Board *board, const int value)
		: start(start), dest(dest), board(board), value(value) {}

	bool operator<(const Move &other) const
	{
		return value < other.value;
	}
};
