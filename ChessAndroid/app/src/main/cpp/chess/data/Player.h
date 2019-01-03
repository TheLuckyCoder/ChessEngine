#pragma once

#include "Board.h"

namespace Player
{

	Pos getKingPos(bool isWhite, const Board &board);
	bool hasOnlyTheKing(bool isWhite, const Board &board);
	bool hasNoMoves(bool isWhite, const Board &board);
	bool hasNoValidMoves(bool isWhite, const Board &board);
	bool isInChess(bool isWhite, const Board &board);
	std::unordered_map<Pos, Piece> getAllOwnedPieces(bool isWhite, const Board &board);

}
