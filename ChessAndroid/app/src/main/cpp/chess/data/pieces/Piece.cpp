#include "Piece.h"

#include "../../BoardManager.h"
#include "PieceEval.h"
#include "MoveGen.h"

Piece::Piece()
	: type(Type::NONE), isWhite(false), hasBeenMoved(false)
{
}

Piece::Piece(const Type type, const bool isWhite, const bool hasBeenMoved)
	: type(type), isWhite(isWhite), hasBeenMoved(hasBeenMoved)
{
}

std::vector<Pos> Piece::getPossibleMoves(const Pos pos, const Board &board) const
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

int Piece::getPoints(const Board &board, const Pos &pos) const
{
	const bool max = isMaximizing();
	const int points = [&]() {
		switch (type)
		{
		case Type::PAWN:
			return PieceEval::evaluatePawn(max, pos, board);
		case Type::KNIGHT:
			return PieceEval::evaluateKnight(max, pos, board);
		case Type::BISHOP:
			return PieceEval::evaluateBishop(max, pos, board);
		case Type::ROOK:
			return PieceEval::evaluateRook(max, pos, board);
		case Type::QUEEN:
			return PieceEval::evaluateQueen(max, pos);
		case Type::KING:
			return PieceEval::evaluateKing(max, pos, board);
		default:
			return 0;
		}
	}();

	return max ? points : -points;
}

bool Piece::hasSameColor(const Piece &other) const
{
	return isWhite == other.isWhite;
}

bool Piece::isMaximizing() const
{
	return isWhite == BoardManager::isWhiteAtBottom;
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
