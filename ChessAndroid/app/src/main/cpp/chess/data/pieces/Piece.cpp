#include "Piece.h"

#include <algorithm>

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

Piece::MovesReturnType Piece::getValidMoves(const Pos &pos, const Board &board) const
{
	auto moves = getPossibleMoves(pos, board);
	if (type == Type::KING)
		return moves;

	const auto iterator = std::remove_if(moves.begin(), moves.end(), [&](const Pos &destPos) {
		Board newBoard = board;
		BoardManager::movePieceInternal(pos, destPos, newBoard, false);
		return Player::isInChess(isWhite, newBoard);
	});
	moves.erase(iterator, moves.end());

	return moves;
}

bool Piece::hasSameColor(const Piece &other) const
{
	return isWhite == other.isWhite;
}
