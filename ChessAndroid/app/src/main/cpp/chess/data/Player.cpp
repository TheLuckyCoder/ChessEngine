#include "Player.h"

#include "Board.h"
#include "pieces/Piece.h"
#include "pieces/MoveGen.h"
#include "../BoardManager.h"

namespace Player
{

	Pos getKingPos(const bool isWhite, const Board &board)
	{
		Pos kingPos;

		for (byte i = 0; i < 8; i++)
			for (byte j = 0; j < 8; j++)
				if (const auto &piece = board.data[i][j]; piece.type == Piece::Type::KING && piece.isWhite == isWhite)
					kingPos = Pos(i, j);

		return kingPos;
	}

	bool hasOnlyTheKing(const bool isWhite, const Board &board)
	{
		short piecesCount = 0;

		for (byte i = 0; i < 8; i++)
			for (byte j = 0; j < 8; j++)
				if (const auto &piece = board.data[i][j]; piece && piece.type != Piece::Type::KING && piece.isWhite == isWhite)
					piecesCount++;

		return piecesCount == 0;
	}

	bool hasNoMoves(const bool isWhite, const Board &board)
	{
		for (byte i = 0; i < 8; i++)
			for (byte j = 0; j < 8; j++)
			{
				const Pos startPos(i, j);
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

	bool hasNoValidMoves(const bool isWhite, const Board &board)
	{
		const auto pieces = getAllOwnedPieces(isWhite, board);

		for (const auto &pair : pieces)
		{
			const auto &startPos = pair.first;
			const auto possibleMoves = pair.second.getPossibleMoves(startPos, board);

			for (const auto &destPos : possibleMoves)
			{
				if (board[destPos].type == Piece::Type::KING)
					continue;

				Board newBoard = board;
				BoardManager::movePieceInternal(startPos, destPos, newBoard);

				if ((isWhite && board.state == GameState::WHITE_IN_CHESS) ||
					(!isWhite && board.state == GameState::BLACK_IN_CHESS))
					continue;

				return false;
			}
		}

		return true;
	}

	bool isInChess(const bool isWhite, const Board &board)
	{
		const auto moves = MoveGen::getAllAttacksPerColor(!isWhite, board);
		const auto iterator = moves.find(getKingPos(isWhite, board));

		return iterator != moves.end();
	}

	std::unordered_map<Pos, Piece> getAllOwnedPieces(const bool isWhite, const Board &board)
	{
		std::unordered_map<Pos, Piece> map;
		map.reserve(16);

		for (byte i = 0; i < 8; i++)
			for (byte j = 0; j < 8; j++)
				if (const auto piece = board.data[i][j]; piece && piece.isWhite == isWhite)
					map[Pos(i, j)] = piece;

		return map;
	}

}
