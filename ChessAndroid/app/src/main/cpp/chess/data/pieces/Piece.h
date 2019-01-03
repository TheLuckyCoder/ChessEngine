#pragma once

#include <vector>

#include "../Pos.h"
#include "../../StackVector.h"

class Board;

class Piece
{
public:
	using MovesReturnType = StackVector<Pos, 27>;

	enum class Type : unsigned char
	{
		NONE = 0,
		PAWN = 1,
		KNIGHT = 2,
		BISHOP = 3,
		ROOK = 4,
		QUEEN = 5,
		KING = 6
	};

	Type type;
	bool isWhite;
	bool hasBeenMoved;

	Piece();
	Piece(Type type, bool isWhite, bool hasBeenMoved = false);
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	MovesReturnType getPossibleMoves(Pos pos, const Board &board) const;
	bool hasSameColor(const Piece &other) const;

	Piece &operator=(const Piece &other);
	Piece &operator=(Piece &&other) = default;

	operator bool() const { return type != Type::NONE; }
};
