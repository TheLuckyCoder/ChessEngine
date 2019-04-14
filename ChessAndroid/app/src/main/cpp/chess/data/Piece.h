#pragma once

#include "Pos.h"
#include "../containers/StackVector.h"

class Board;

enum Type : unsigned char
{
	NONE = 0,
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

class Piece final
{
public:
	const static Piece EMPTY;

	using MaxMovesVector = StackVector<Pos, 27>;

	Type type;
	bool isWhite;
	bool moved;

	constexpr Piece() noexcept
		: type(Type::NONE), isWhite(false), moved(false) {}
	constexpr Piece(const Type type, const bool isWhite, const bool moved = false) noexcept
		: type(type), isWhite(isWhite), moved(moved) {}
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	Piece &operator=(const Piece &other) = default;
	Piece &operator=(Piece &&other) = default;

	MaxMovesVector getPossibleMoves(const Pos &pos, const Board &board) const noexcept;
	MaxMovesVector getPossibleCaptures(const Pos &pos, const Board &board) const noexcept;

	/*
	 * Checks if the type and color match
	 */
	constexpr bool isSameType(const Piece &other) const noexcept
	{
		return type == other.type && isWhite == other.isWhite;
	}

	constexpr bool isSameColor(const Piece &other) const noexcept
	{
		return isWhite == other.isWhite;
	}

	constexpr operator bool() const noexcept
	{
		return type != Type::NONE;
	}
};
