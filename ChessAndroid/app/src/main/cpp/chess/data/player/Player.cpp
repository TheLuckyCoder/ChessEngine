#include "Player.h"

#include <math.h>
#include "../pieces/Piece.h"

Player::Player(bool isWhite)
	: isWhite(isWhite)
{
}

bool Player::hasOnlyTheKing(const Board &board) const
{
	short piecesCount = 0;

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (Piece *piece = board.data[i][j]; piece && piece->isWhite == isWhite)
				piecesCount++;

	return piecesCount == 0;
}

std::unordered_map<Pos, Piece*> Player::getAllOwnedPieces(const bool isWhite, const Board &board)
{
	std::unordered_map<Pos, Piece*> map;
	map.reserve(16);

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
		{
			Piece *piece = board.data[i][j];
			if (piece && piece->isWhite == isWhite)
				map[Pos(i, j)] = piece;
		}

	return map;
}
