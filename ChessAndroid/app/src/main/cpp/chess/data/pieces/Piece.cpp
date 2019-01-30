#include "Piece.h"

#include "MoveGen.h"
#include "../Board.h"
#include "../../BoardManager.h"

Piece::Piece()
	: type(Type::NONE), isWhite(false), moved(false) {}

Piece::Piece(const Type type, const bool isWhite, const bool hasBeenMoved)
	: type(type), isWhite(isWhite), moved(hasBeenMoved) {}

Piece::MovesReturnType Piece::getPossibleMoves(const Pos &pos, const Board &board) const
{
	MovesReturnType result;

	switch (type)
	{
	case Type::NONE:
		break;
	case Type::PAWN:
		result = MoveGen<ALL>::generatePawnMoves(*this, pos, board);
		break;
	case Type::KNIGHT:
		result = MoveGen<ALL>::generateKnightMoves(*this, pos, board);
		break;
	case Type::BISHOP:
		result = MoveGen<ALL>::generateBishopMoves(*this, pos, board);
		break;
	case Type::ROOK:
		result = MoveGen<ALL>::generateRookMoves(*this, pos, board);
		break;
	case Type::QUEEN:
		result = MoveGen<ALL>::generateQueenMoves(*this, pos, board);
		break;
	case Type::KING:
		result = MoveGen<ALL>::generateKingMoves(*this, pos, board);
		break;
	}

	return result;
}

bool Piece::hasSameColor(const Piece &other) const
{
	return isWhite == other.isWhite;
}
