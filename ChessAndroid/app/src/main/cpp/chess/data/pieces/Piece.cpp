#include "Piece.h"

#include "../../BoardManager.h"
#include "PieceEval.h"
#include "MoveGen.h"

std::vector<Pos> Piece::getPossibleMoves(Pos pos, const Board &board) const
{
	std::vector<Pos> moves;

	switch (type)
	{
	case Type::NONE:
		break;
	case Type::PAWN:
		MoveGen::generatePawnMoves(*this, pos, moves, board);
		break;
	case Type::KNIGHT:
		MoveGen::generateKnightMoves(*this, pos, moves, board);
		break;
	case Type::BISHOP:
		MoveGen::generateBishopMoves(*this, pos, moves, board);
		break;
	case Type::ROOK:
		MoveGen::generateRookMoves(*this, pos, moves, board);
		break;
	case Type::QUEEN:
		MoveGen::generateQueenMoves(*this, pos, moves, board);
		break;
	case Type::KING:
		MoveGen::generateKingMoves(*this, pos, moves, board);
		break;
	}

	return moves;
}

int Piece::getPoints(const int x, const int y) const
{
	const int points = evaluatePiece(x, y);
	return isMaximizing() ? points : -points;
}

bool Piece::hasSameColor(const Piece &other) const
{
	return isWhite == other.isWhite;
}

bool Piece::isMaximizing() const
{
	return isWhite == BoardManager::isWhiteAtBottom;
}

int Piece::evaluatePiece(const int x, const int y) const
{
	switch (type)
	{
	case Type::NONE:
		return 0;
	case Type::PAWN:
		return PieceEval::PAWN + (isMaximizing() ? PieceEval::PAWN_WHITE[x][y] : PieceEval::PAWN_BLACK[x][y]);
	case Type::KNIGHT:
		return PieceEval::KNIGHT + (isMaximizing() ? PieceEval::KNIGHT_WHITE[x][y] : PieceEval::KNIGHT_BLACK[x][y]);
	case Type::BISHOP:
		return PieceEval::BISHOP + (isMaximizing() ? PieceEval::BISHOP_WHITE[x][y] : PieceEval::BISHOP_BLACK[x][y]);
	case Type::ROOK:
		return PieceEval::ROOK + (isMaximizing() ? PieceEval::ROOK_WHITE[x][y] : PieceEval::ROOK_BLACK[x][y]);
	case Type::QUEEN:
		return PieceEval::QUEEN + (isMaximizing() ? PieceEval::QUEEN_WHITE[x][y] : PieceEval::QUEEN_BLACK[x][y]);
	case Type::KING:
		return PieceEval::KING + (isMaximizing() ? PieceEval::KING_WHITE[x][y] : PieceEval::KING_BLACK[x][y]);
	}
	return 0;
}

Piece &Piece::operator=(const Piece &other)
{
	if (this != &other)
	{
		type = other.type;
		isWhite = other.isWhite;
		hasBeenMoved = other.hasBeenMoved;
	}
	return *this;
}
