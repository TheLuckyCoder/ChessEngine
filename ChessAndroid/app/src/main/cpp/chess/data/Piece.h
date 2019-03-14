#pragma once

#include "Pos.h"
#include "../containers/StackVector.h"

class Board;

class Piece final
{
public:
	using MaxMovesVector = StackVector<Pos, 27>;

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
	bool moved;

	constexpr Piece() noexcept
		: type(Type::NONE), isWhite(false), moved(false) {}
	constexpr Piece(const Type type, const bool isWhite, const bool hasBeenMoved = false) noexcept
		: type(type), isWhite(isWhite), moved(hasBeenMoved) {}
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	Piece &operator=(const Piece &other) = default;
	Piece &operator=(Piece &&other) = default;

	MaxMovesVector getPossibleMoves(const Pos &pos, const Board &board) const noexcept;
	MaxMovesVector getPossibleCaptures(const Pos &pos, const Board &board) const noexcept;

	constexpr bool hasSameColor(const Piece &other) const noexcept
	{
		return isWhite == other.isWhite;
	}

	constexpr operator bool() const noexcept
	{
		return type != Type::NONE;
	}
};

constexpr byte toByte(Piece::Type type)
{
	return static_cast<byte>(type);
}
