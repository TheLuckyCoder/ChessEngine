#include "Piece.h"

#include "../algorithm/MoveGen.h"

Piece::MaxMovesVector Piece::getPossibleMoves(const Pos &pos, const Board &board) const noexcept
{
	MaxMovesVector result;

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

Piece::MaxMovesVector Piece::getPossibleCaptures(const Pos &pos, const Board &board) const noexcept
{
	MaxMovesVector result;

	switch (type)
	{
	case Type::NONE:
		break;
	case Type::PAWN:
		result = MoveGen<CAPTURES>::generatePawnMoves(*this, pos, board);
		break;
	case Type::KNIGHT:
		result = MoveGen<CAPTURES>::generateKnightMoves(*this, pos, board);
		break;
	case Type::BISHOP:
		result = MoveGen<CAPTURES>::generateBishopMoves(*this, pos, board);
		break;
	case Type::ROOK:
		result = MoveGen<CAPTURES>::generateRookMoves(*this, pos, board);
		break;
	case Type::QUEEN:
		result = MoveGen<CAPTURES>::generateQueenMoves(*this, pos, board);
		break;
	case Type::KING:
		result = MoveGen<CAPTURES>::generateKingMoves(*this, pos, board);
		break;
	}

	return result;
}
