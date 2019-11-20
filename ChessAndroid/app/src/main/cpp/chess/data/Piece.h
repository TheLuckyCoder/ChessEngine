#pragma once

#include "Pos.h"
#include "../containers/StackVector.h"

class Board;

enum PieceType : unsigned char
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

	PieceType type;
	bool isWhite;

	constexpr Piece() noexcept
		: type(PieceType::NONE), isWhite(false) {}
	constexpr Piece(const PieceType type, const bool isWhite) noexcept
		: type(type), isWhite(isWhite) {}
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	Piece &operator=(const Piece &other) = default;
	Piece &operator=(Piece &&other) = default;

	MaxMovesVector getPossibleMoves(const byte square, const Board &board) const noexcept;
	MaxMovesVector getPossibleCaptures(const byte square, const Board &board) const noexcept;

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
		return type != PieceType::NONE;
	}
};
