#pragma once

#include "Pos.h"
#include "Board.h"

class Move
{
public:
	Pos start;
	Pos dest;
	Board board;
	int value;

	Move(const Pos start, const Pos dest, Board &&board, const int value)
		: start(start), dest(dest), board(std::move(board)), value(value) {}

	bool operator<(const Move &other) const
	{
		return value < other.value;
	}
};
