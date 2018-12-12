#pragma once

#include "Piece.h"

class RookPiece : public Piece
{
public:
	explicit RookPiece(bool isWhite)
		: Piece(Type::ROOK, isWhite) {}

	explicit RookPiece(const Piece *piece)
		: Piece(piece) {}

private:
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override
	{
		Pos posCopy = pos;
		while (posCopy.x > 0)
		{
			posCopy.x--;

			auto *other = board[posCopy];

			if (other)
			{
				if (!hasSameColor(*other))
					moves.push_back(posCopy);
				break;
			}
			else moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x < 7)
		{
			posCopy.x++;

			auto *other = board[posCopy];

			if (other)
			{
				if (!hasSameColor(*other))
					moves.push_back(posCopy);
				break;
			}
			else moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.y > 0)
		{
			posCopy.y--;

			auto *other = board[posCopy];

			if (other)
			{
				if (!hasSameColor(*other))
					moves.push_back(posCopy);
				break;
			}
			else moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.y < 7)
		{
			posCopy.y++;

			auto *other = board[posCopy];

			if (other)
			{
				if (!hasSameColor(*other))
					moves.push_back(posCopy);
				break;
			}
			else moves.push_back(posCopy);
		}
	}

	int evaluatePiece(const int x, const int y) const override
	{
		return PieceEval::ROOK + (isMaximising() ? PieceEval::ROOK_WHITE[x][y] : PieceEval::ROOK_BLACK[x][y]);
	}
};
