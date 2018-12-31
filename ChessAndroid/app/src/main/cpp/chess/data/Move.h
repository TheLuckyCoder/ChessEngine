#pragma once

#include "Pos.h"
#include "Board.h"
#include "../GameState.h"

class Move
{
public:
	Pos start;
	Pos dest;
	Board board;
	int value;
	GameState state;

	Move() = default;

	Move(const Pos start, const Pos dest, Board &&board, const int value, const GameState state)
		: start(start), dest(dest), board(std::move(board)), value(value), state(state) {}

	bool operator<(const Move &other) const
	{
		return value < other.value;
	}
};
