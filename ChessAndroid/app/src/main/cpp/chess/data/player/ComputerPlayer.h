#pragma once

#include <utility>

#include "Player.h"

class ComputerPlayer : public Player
{
public:
	ComputerPlayer()
		: Player(false) {}

	std::pair<Pos, Pos> getNextMove() const;
};
