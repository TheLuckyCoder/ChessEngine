#include "Piece.h"

#include "../algorithm/MoveGen.h"

const Piece Piece::EMPTY{};

U64 Piece::getPossibleMoves(const byte square, const Board &board) const noexcept
{
	U64 result{};

	switch (type)
	{
	case PieceType::NONE:
		break;
	case PieceType::PAWN:
		result = MoveGen<ALL>::generatePawnMoves(*this, square, board);
		break;
	case PieceType::KNIGHT:
		result = MoveGen<ALL>::generateKnightMoves(*this, square, board);
		break;
	case PieceType::BISHOP:
		result = MoveGen<ALL>::generateBishopMoves(*this, square, board);
		break;
	case PieceType::ROOK:
		result = MoveGen<ALL>::generateRookMoves(*this, square, board);
		break;
	case PieceType::QUEEN:
		result = MoveGen<ALL>::generateQueenMoves(*this, square, board);
		break;
	case PieceType::KING:
		result = MoveGen<ALL>::generateKingMoves(*this, square, board);
		break;
	}

	return result;
}

U64 Piece::getPossibleCaptures(const byte square, const Board &board) const noexcept
{
	U64 result{};

	switch (type)
	{
	case PieceType::NONE:
		break;
	case PieceType::PAWN:
		result = MoveGen<CAPTURES>::generatePawnMoves(*this, square, board);
		break;
	case PieceType::KNIGHT:
		result = MoveGen<CAPTURES>::generateKnightMoves(*this, square, board);
		break;
	case PieceType::BISHOP:
		result = MoveGen<CAPTURES>::generateBishopMoves(*this, square, board);
		break;
	case PieceType::ROOK:
		result = MoveGen<CAPTURES>::generateRookMoves(*this, square, board);
		break;
	case PieceType::QUEEN:
		result = MoveGen<CAPTURES>::generateQueenMoves(*this, square, board);
		break;
	case PieceType::KING:
		result = MoveGen<CAPTURES>::generateKingMoves(*this, square, board);
		break;
	}

	return result;
}
