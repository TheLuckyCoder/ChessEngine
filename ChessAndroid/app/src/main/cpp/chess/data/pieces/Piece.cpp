#include "Piece.h"

#include "../../BoardManager.h"

std::vector<Pos> Piece::getPossibleMoves(Pos pos, const Board &board) const
{
	std::vector<Pos> moves;

	calculateMoves(pos, moves, board);

	return moves;
}

int Piece::getPoints(const int x, const int y) const
{
	const int points = evaluatePiece(x, y);
	return isMaximising() ? points : -points;
}

bool Piece::hasSameColor(const Piece &other) const
{
	return isWhite == other.isWhite;
}

bool Piece::isMaximising() const
{
	return isWhite == BoardManager::isWhiteAtBottom;
}

bool Piece::operator==(const Piece &other) const
{
	return type == other.type && isWhite == other.isWhite;
}

bool Piece::operator!=(const Piece &other) const
{
	return !(*this == other);
}
