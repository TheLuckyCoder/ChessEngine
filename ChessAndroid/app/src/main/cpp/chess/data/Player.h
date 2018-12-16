#pragma once

#include "Board.h"

namespace Player
{
	bool hasOnlyTheKing(bool isWhite, const Board &board);
	std::unordered_map<Pos, Piece> getAllOwnedPieces(bool isWhite, const Board &board);
}
