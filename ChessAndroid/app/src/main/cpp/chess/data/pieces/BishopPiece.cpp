#include "BishopPiece.h"

void BishopPiece::calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const
{
	Pos posCopy = pos;
	while (posCopy.x < 7 && posCopy.y > 0)
	{
		posCopy.x++;
		posCopy.y--;

		auto *other = board[posCopy];

		if (other)
		{
			if (!hasSameColor(*other))
				moves.push_back(posCopy);
			break;
		}
		else
			moves.push_back(posCopy);
	}

	posCopy = pos;
	while (posCopy.x < 7 && posCopy.y < 7)
	{
		posCopy.x++;
		posCopy.y++;

		auto *other = board[posCopy];

		if (other)
		{
			if (!hasSameColor(*other))
				moves.push_back(posCopy);
			break;
		}
		else
			moves.push_back(posCopy);
	}

	posCopy = pos;
	while (posCopy.x > 0 && posCopy.y > 0)
	{
		posCopy.x--;
		posCopy.y--;

		auto *other = board[posCopy];

		if (other)
		{
			if (!hasSameColor(*other))
				moves.push_back(posCopy);
			break;
		}
		else
			moves.push_back(posCopy);
	}

	posCopy = pos;
	while (posCopy.x > 0 && posCopy.y < 7)
	{
		posCopy.x--;
		posCopy.y++;

		auto *other = board[posCopy];

		if (other)
		{
			if (!hasSameColor(*other))
				moves.push_back(posCopy);
			break;
		}
		else
			moves.push_back(posCopy);
	}
}

float BishopPiece::evaluatePiece(const int x, const int y) const
{
	return PieceEval::BISHOP + (isMaximising() ? PieceEval::BISHOP_WHITE[x][y] : PieceEval::BISHOP_BLACK[x][y]);
}
