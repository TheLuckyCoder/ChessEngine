#pragma once

#include "../containers/StackVector.h"
#include "../data/Pos.h"

class Piece;
class Board;

class Player
{
public:

	Player() = delete;
	Player(const Player&) = delete;
	Player(Player&&) = delete;

	static bool onlyKingsLeft(const Board &board);
	static bool hasNoValidMoves(bool isWhite, const Board &board);
	static bool isInCheck(bool isWhite, const Board &board);
	static StackVector<std::pair<Pos, Piece>, 16> getAllOwnedPieces(bool isWhite, const Board &board);
};
