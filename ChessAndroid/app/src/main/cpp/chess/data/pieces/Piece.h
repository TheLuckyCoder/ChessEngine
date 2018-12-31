#pragma once

#include <vector>

#include "../Pos.h"

class Board;

class Piece
{
public:
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

	std::vector<Pos> getPossibleMoves(Pos pos, const Board &board) const;
	int getPoints(const Board &board, const Pos &pos) const;
	bool hasSameColor(const Piece &other) const;

	Piece &operator=(const Piece &other);
	Piece &operator=(Piece &&other) = default;

	operator bool() const { return type != Type::NONE; }

private:
	bool isMaximizing() const;
};
