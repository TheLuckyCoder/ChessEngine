#pragma once

#include "Pos.h"

class Board;

class Piece final
{
public:
	const static Piece EMPTY;

	PieceType type;
	Color color;

	constexpr Piece() noexcept
		: type(PieceType::NO_PIECE_TYPE), color(BLACK) {}
	constexpr Piece(const PieceType type, const Color color) noexcept
		: type(type), color(color) {}
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	Piece &operator=(const Piece &other) = default;
	Piece &operator=(Piece &&other) = default;

	constexpr bool operator==(const Piece &other) const noexcept
	{
		return type == other.type && color == other.color;
	}

	U64 getPossibleMoves(const byte square, const Board &board) const noexcept;
	U64 getPossibleCaptures(const byte square, const Board &board) const noexcept;

	constexpr bool isSameColor(const Piece &other) const noexcept
	{
		return color == other.color;
	}

	constexpr operator bool() const noexcept
	{
		return type != PieceType::NO_PIECE_TYPE;
	}
};
