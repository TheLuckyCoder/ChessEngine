#pragma once

#include "../memory/StackVector.h"
#include "../data/Pos.h"

class Piece;
class Board;

namespace Player
{

	Pos getKingPos(bool isWhite, const Board &board);
	bool onlyKingsLeft(const Board &board);
	bool hasNoMoves(bool isWhite, const Board &board);
	bool hasNoValidMoves(bool isWhite, const Board &board);
	bool isInChess(bool isWhite, const Board &board);
	StackVector<std::pair<Pos, Piece>, 16> getAllOwnedPieces(bool isWhite, const Board &board);

}
