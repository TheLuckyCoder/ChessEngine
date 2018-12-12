#include "PawnPiece.h"

void PawnPiece::getInitialMoves(Pos pos, std::vector<Pos> &moves, const Board &board) const
{
	isWhite ? pos.y++ : pos.y--;
	if (!board[pos])
	{
		moves.push_back(pos);

		if (!hasBeenMoved) {
			Pos posCopy = pos;

			isWhite ? posCopy.y++ : posCopy.y--;
			if (!board[posCopy])
				moves.push_back(posCopy);
		}
	}
}

void PawnPiece::addAttackMoves(Pos pos, std::vector<Pos> &moves) const
{
	isWhite ? pos.y++ : pos.y--;

	isWhite ? pos.x-- : pos.x++;
	if (pos.isValid())
		moves.push_back(pos);

	pos.x += isWhite ? 2 : -2;
	if (pos.isValid())
		moves.push_back(pos);
}

void PawnPiece::calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const
{
	getInitialMoves(pos, moves, board);

	isWhite ? pos.y++ : pos.y--;

	isWhite ? pos.x-- : pos.x++;
	if (pos.isValid())
	{
		auto *other = board[pos];

		if (other && !hasSameColor(*other))
			moves.push_back(pos);
	}

	pos.x += isWhite ? 2 : -2;
	if (pos.isValid())
	{
		auto *other = board[pos];

		if (other && !hasSameColor(*other))
			moves.push_back(pos);
	}
}

int PawnPiece::evaluatePiece(const int x, const int y) const
{
	return PieceEval::PAWN + (isMaximising() ? PieceEval::PAWN_WHITE[x][y] : PieceEval::PAWN_BLACK[x][y]);
}
