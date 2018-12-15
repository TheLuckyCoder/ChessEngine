#pragma once

#include "../Board.h"

class Player
{
public:
	const bool isWhite;

	explicit Player(bool isWhite);

	bool hasOnlyTheKing(const Board &board) const;
	static std::unordered_map<Pos, Piece*> getAllOwnedPieces(bool isWhite, const Board &board);
};
