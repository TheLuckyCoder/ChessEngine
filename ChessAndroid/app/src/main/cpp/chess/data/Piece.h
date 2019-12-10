#pragma once

#include "Pos.h"
#include "../containers/StackVector.h"

class Board;

class Piece final
{
public:
	const static Piece EMPTY;

	PieceType type;
	bool isWhite;

	constexpr Piece() noexcept
		: type(PieceType::NO_PIECE_TYPE), isWhite(false) {}
	constexpr Piece(const PieceType type, const bool isWhite) noexcept
		: type(type), isWhite(isWhite) {}
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	Piece &operator=(const Piece &other) = default;
	Piece &operator=(Piece &&other) = default;

	constexpr bool operator==(const Piece &other) const noexcept
	{
		return type == other.type && isWhite == other.isWhite;
	}

	U64 getPossibleMoves(const byte square, const Board &board) const noexcept;
	U64 getPossibleCaptures(const byte square, const Board &board) const noexcept;

	constexpr bool isSameColor(const Piece &other) const noexcept
	{
		return isWhite == other.isWhite;
	}

	constexpr operator bool() const noexcept
	{
		return type != PieceType::NO_PIECE_TYPE;
	}
};
