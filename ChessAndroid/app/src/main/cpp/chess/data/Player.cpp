#include "Player.h"

#include "pieces/Piece.h"

namespace Player
{
	bool hasOnlyTheKing(const bool isWhite, const Board &board)
	{
		short piecesCount = 0;

		for (short i = 0; i < 8; i++)
			for (short j = 0; j < 8; j++)
				if (const auto &piece = board.data[i][j]; piece && piece.isWhite == isWhite)
					piecesCount++;

		return piecesCount == 0;
	}

	std::unordered_map<Pos, Piece> getAllOwnedPieces(const bool isWhite, const Board &board)
	{
		std::unordered_map<Pos, Piece> map;
		map.reserve(16);

		for (short i = 0; i < 8; i++)
			for (short j = 0; j < 8; j++)
				if (const auto piece = board.data[i][j]; piece && piece.isWhite == isWhite)
					map[Pos(i, j)] = piece;

		return map;
	}

}
