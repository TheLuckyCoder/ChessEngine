#include "Piece.h"

#include "MoveGen.h"

Piece::Piece()
	: type(Type::NONE), isWhite(false), hasBeenMoved(false)
{
}

Piece::Piece(const Type type, const bool isWhite, const bool hasBeenMoved)
	: type(type), isWhite(isWhite), hasBeenMoved(hasBeenMoved)
{
}

Piece::MovesReturnType Piece::getPossibleMoves(const Pos pos, const Board &board) const
{
	MovesReturnType result;

	switch (type)
	{
	case Type::NONE:
		break;
	case Type::PAWN:
		result = MoveGen::generatePawnMoves(*this, pos, board);
		break;
	case Type::KNIGHT:
		result = MoveGen::generateKnightMoves(*this, pos, board);
		break;
	case Type::BISHOP:
		result = MoveGen::generateBishopMoves(*this, pos, board);
		break;
	case Type::ROOK:
		result = MoveGen::generateRookMoves(*this, pos, board);
		break;
	case Type::QUEEN:
		result = MoveGen::generateQueenMoves(*this, pos, board);
		break;
	case Type::KING:
		result = MoveGen::generateKingMoves(*this, pos, board);
		break;
	}

	return result;
}

bool Piece::hasSameColor(const Piece &other) const
{
	return isWhite == other.isWhite;
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