#include "Piece.h"

#include "../algorithm/MoveGen.h"

const Piece Piece::EMPTY{};

U64 Piece::getPossibleMoves(const byte square, const Board &board) const noexcept
{
	const Color c = color();
	U64 result{};

	switch (type())
	{
	case PieceType::PAWN:
		result = MoveGen<ALL>::getPawnMoves(c, square, board);
		break;
	case PieceType::KNIGHT:
		result = MoveGen<ALL>::getKnightMoves(c, square, board);
		break;
	case PieceType::BISHOP:
		result = MoveGen<ALL>::getBishopMoves(c, square, board);
		break;
	case PieceType::ROOK:
		result = MoveGen<ALL>::getRookMoves(c, square, board);
		break;
	case PieceType::QUEEN:
		result = MoveGen<ALL>::getQueenMoves(c, square, board);
		break;
	case PieceType::KING:
		result = MoveGen<ALL>::getKingMoves(c, square, board);
		break;
	default:
		break;
	}
	
	return result;
}

U64 Piece::getPossibleCaptures(const byte square, const Board &board) const noexcept
{
	const Color c = color();
	U64 result{};

	switch (type())
	{
	case PieceType::PAWN:
		result = MoveGen<CAPTURES>::getPawnMoves(c, square, board);
		break;
	case PieceType::KNIGHT:
		result = MoveGen<CAPTURES>::getKnightMoves(c, square, board);
		break;
	case PieceType::BISHOP:
		result = MoveGen<CAPTURES>::getBishopMoves(c, square, board);
		break;
	case PieceType::ROOK:
		result = MoveGen<CAPTURES>::getRookMoves(c, square, board);
		break;
	case PieceType::QUEEN:
		result = MoveGen<CAPTURES>::getQueenMoves(c, square, board);
		break;
	case PieceType::KING:
		result = MoveGen<CAPTURES>::getKingMoves(c, square, board);
		break;
	default:
		break;
	}

	return result;
}
