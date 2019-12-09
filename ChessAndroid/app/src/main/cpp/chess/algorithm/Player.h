#pragma once

#include "../containers/Containers.h"
#include "Defs.h"
#include "../data/Pos.h"

class Piece;
class Board;

class Player
{
public:
	Player() = delete;
	Player(const Player&) = delete;
	Player(Player&&) = delete;

	static bool isAttacked(Color colorAttacking, byte targetSquare, const Board &board);
	static bool onlyKingsLeft(const Board &board);
	static bool hasNoValidMoves(Color color, const Board &board);
	static bool isInCheck(Color color, const Board &board);
	static StackVector<std::pair<byte, Piece>, 32> getAllOwnedPieces(Color color, const Board &board);
	static AttacksMap getAttacksPerColor(bool white, const Board &board);
};
