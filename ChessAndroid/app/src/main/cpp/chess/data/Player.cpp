#include "Player.h"

#include "pieces/Piece.h"
#include "Move.h"
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

				if (piece && piece.isWhite == isWhite)
				{
					const auto possibleMoves = piece.getPossibleMoves(startPos, board);

					for (auto &destPos : possibleMoves)
					{
						Board newBoard = board;
						const auto state = BoardManager::movePieceInternal(startPos, destPos, newBoard, false);

						if ((isWhite && state == GameState::WHITE_IN_CHESS) ||
							(!isWhite && state == GameState::BLACK_IN_CHESS))
							continue;

						return false;
					}
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
				Board newBoard = board;
				const auto state = BoardManager::movePieceInternal(startPos, destPos, newBoard);

				if ((isWhite && state == GameState::WHITE_IN_CHESS) ||
					(!isWhite && state == GameState::BLACK_IN_CHESS))
					continue;

				return false;
			}
		}

		return true;
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

		for (byte i = 0; i < 8; i++)
			for (byte j = 0; j < 8; j++)
				if (const auto piece = board.data[i][j]; piece && piece.isWhite == isWhite)
					map[Pos(i, j)] = piece;

		return map;
	}

}
