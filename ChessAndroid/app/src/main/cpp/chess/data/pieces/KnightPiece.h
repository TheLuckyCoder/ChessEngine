#pragma once

#include "Piece.h"

class KnightPiece : public Piece
{
public:
	explicit KnightPiece(bool isWhite)
		: Piece(Type::KNIGHT, isWhite) {}

	explicit KnightPiece(const Piece *piece)
		: Piece(piece) {}

private:
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override
	{
		auto addPosIfValid = [&](const short x, const short y) {
			Pos startPos = Pos(pos.x + x, pos.y + y);

			if (startPos.isValid())
			{
				auto *other = board[startPos];

				if (!other || !hasSameColor(*other))
					moves.push_back(startPos);
			}
		};

		addPosIfValid(-1, 2);
		addPosIfValid(1, 2);

		addPosIfValid(-1, -2);
		addPosIfValid(1, -2);

		addPosIfValid(2, 1);
		addPosIfValid(2, -1);

		addPosIfValid(-2, 1);
		addPosIfValid(-2, -1);
	}

	float evaluatePiece(const int x, const int y) const override
	{
		return PieceEval::KNIGHT + (isMaximising() ? PieceEval::KNIGHT_WHITE[x][y] : PieceEval::KNIGHT_BLACK[x][y]);
	}
};
