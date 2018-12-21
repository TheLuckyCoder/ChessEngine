#pragma once

#include <vector>

#include "../Pos.h"

class Board;

class Piece
{
public:
	enum class Type : unsigned char
	{
		NONE,
		PAWN,
		KNIGHT,
		BISHOP,
		ROOK,
		QUEEN,
		KING
	};

	Type type;
	bool isWhite;
	bool hasBeenMoved = false;

	Piece()
		: type(Type::NONE), isWhite(false) {}
	Piece(const Type type, const bool isWhite)
		: type(type), isWhite(isWhite) {}
	Piece(const Type type, const bool isWhite, const bool hasBeenMoved)
		: type(type), isWhite(isWhite), hasBeenMoved(hasBeenMoved) {}
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	std::vector<Pos> getPossibleMoves(Pos pos, const Board &board) const;
	int getPoints(int x, int y) const;
	bool hasSameColor(const Piece &other) const;

	Piece &operator=(const Piece &other);
	Piece &operator=(Piece &&other) = default;

	operator bool() const { return type != Type::NONE; }

private:
	bool isMaximizing() const;

	int evaluatePiece(int x, int y) const;
};
