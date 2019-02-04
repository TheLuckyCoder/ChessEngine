#include "Player.h"

#include "Board.h"
#include "pieces/Piece.h"
#include "pieces/MoveGen.h"
#include "../BoardManager.h"

namespace Player
{

	Pos getKingPos(const bool isWhite, const Board &board)
	{
		for (byte x = 0; x < 8; x++)
			for (byte y = 0; y < 8; y++)
				if (const auto &piece = board.data[x][y];
					piece.type == Piece::Type::KING && piece.isWhite == isWhite)
					return Pos(x, y);

		return Pos();
	}

	bool onlyKingsLeft(const Board &board)
	{
		for (byte x = 0; x < 8; x++)
			for (byte y = 0; y < 8; y++)
				if (const auto &piece = board.data[x][y];
					piece && piece.type != Piece::Type::KING)
					return false;

		return true;
	}

	bool hasNoValidMoves(const bool isWhite, const Board &board)
	{
		for (byte x = 0; x < 8; x++)
			for (byte y = 0; y < 8; y++)
			{
				const Pos startPos(x, y);
				const auto &piece = board[startPos];
				const auto possibleMoves = piece.getPossibleMoves(startPos, board);

				for (const auto &destPos : possibleMoves)
				{
					if (board[destPos].type == Piece::Type::KING)
						continue;

					Board newBoard = board;
					BoardManager::movePieceInternal(startPos, destPos, newBoard, false);

					if ((isWhite && isInChess(true, newBoard)) ||
						(!isWhite && isInChess(false, newBoard)))
						continue;

					return false;
				}
			}

		return true;
	}

	bool isInChess(const bool isWhite, const Board &board)
	{
		const auto moves = MoveGen<CAPTURES>::getAttacksPerColor(!isWhite, board);
		return exists(moves, getKingPos(isWhite, board));
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
