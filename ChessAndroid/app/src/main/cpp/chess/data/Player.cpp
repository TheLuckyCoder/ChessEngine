#include "Player.h"

#include "Board.h"
#include "Piece.h"
#include "../algorithm/MoveGen.h"
#include "../BoardManager.h"

namespace Player
{

	bool onlyKingsLeft(const Board &board)
	{
		for (byte x = 0; x < 8; x++)
			for (byte y = 0; y < 8; y++)
				if (const auto &piece = board.data[x][y];
					piece && piece.type != Type::KING)
					return false;

		return true;
	}

	bool hasNoValidMoves(const bool isWhite, const Board &board)
	{
		const auto pieces = getAllOwnedPieces(isWhite, board);

		for (const auto &pair : pieces)
		{
			const auto &startPos = pair.first;
			const auto &selectedPiece = pair.second;
			const auto possibleMoves = selectedPiece.getPossibleMoves(startPos, board);

			for (const auto &destPos : possibleMoves)
			{
				if (board[destPos].type == Type::KING)
					continue;

				Board newBoard = board;
				BoardManager::movePieceInternal(startPos, destPos, newBoard, false);

				if (isInChess(isWhite, newBoard))
					continue;

				return false;
			}
		}

		return true;
	}

	bool isInChess(const bool isWhite, const Board &board)
	{
		const Bitboard bitboard = MoveGen<CAPTURES>::getAttacksPerColorBitboard(!isWhite, board);
		const Bitboard kingPos = isWhite ? board.whiteKingPos : board.blackKingPos;
		return static_cast<bool>(bitboard & kingPos);
	}

	StackVector<std::pair<Pos, Piece>, 16> getAllOwnedPieces(const bool isWhite, const Board &board)
	{
		StackVector<std::pair<Pos, Piece>, 16> pieces;

		for (byte x = 0; x < 8; x++)
			for (byte y = 0; y < 8; y++)
				if (const auto &piece = board.data[x][y]; piece && piece.isWhite == isWhite)
					pieces.emplace_back(Pos(x, y), piece);

		return pieces;
	}

}
