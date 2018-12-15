#include "RookPiece.h"

#include "PieceEval.h"

void RookPiece::calculateMoves(Pos & pos, std::vector<Pos>& moves, const Board & board) const
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
		moves.push_back(posCopy);
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
		moves.push_back(posCopy);
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
		moves.push_back(posCopy);
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
		moves.push_back(posCopy);
	}
}

int RookPiece::evaluatePiece(const int x, const int y) const
{
	return PieceEval::ROOK + (isMaximising() ? PieceEval::ROOK_WHITE[x][y] : PieceEval::ROOK_BLACK[x][y]);
}
