#include "Player.h"

#include "pieces/Piece.h"
#include "Move.h"
#include "pieces/MoveGen.h"

namespace Player
{

	Pos getKingPos(const bool isWhite, const Board &board)
	{
		Pos kingPos;

		for (short i = 0; i < 8; i++)
			for (short j = 0; j < 8; j++)
				if (const auto &piece = board.data[i][j]; piece.type == Piece::Type::KING && piece.isWhite == isWhite)
					kingPos = Pos(i, j);

		return kingPos;
	}

	bool hasOnlyTheKing(const bool isWhite, const Board &board)
	{
		short piecesCount = 0;

		for (short i = 0; i < 8; i++)
			for (short j = 0; j < 8; j++)
				if (const auto &piece = board.data[i][j]; piece && piece.isWhite == isWhite)
					piecesCount++;

		return piecesCount == 0;
	}

	bool hasNoMoves(const bool isWhite, const Board &board)
	{
		return board.listMoves(isWhite).empty();
	}

	bool hasNoValidMoves(const bool isWhite, const Board &board)
	{
		return board.listValidMoves(isWhite).empty();
	}

	bool isInChess(const bool isWhite, const Board &board)
	{
		const auto moves = MoveGen::getAllMovesPerColor(!isWhite, board);
		const auto iterator = moves.find(getKingPos(isWhite, board));

		return iterator != moves.end();
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
