#pragma once

#include "Defs.h"

class Piece final
{
public:
	Piece() noexcept = default;
	
	constexpr Piece(const PieceType type, const Color color) noexcept
		: _piece((color << 3u) | type) {}

	explicit constexpr Piece(const u8 type) noexcept
		: _piece(type) {}
	
	Piece(Piece&&) noexcept = default;
	Piece(const Piece&) noexcept = default;
	~Piece() noexcept = default;

	Piece &operator=(const Piece &other) = default;
	Piece &operator=(Piece &&other) = default;

	constexpr bool operator==(const Piece &other) const noexcept
	{
		return _piece == other._piece;
	}

	constexpr bool operator==(const PieceType pieceType) const noexcept
	{
		return type() == pieceType;
	}

	constexpr Color color() const noexcept
	{
		return static_cast<Color>(_piece >> 3u);
	}

	constexpr PieceType type() const noexcept
	{
		return static_cast<PieceType>(_piece & 7u);
	}

	constexpr bool isValid() const noexcept
	{
		return isValid(type());
	}

	/*
	 * Flip the color of the piece
	 */
	constexpr Piece operator~() const noexcept
	{
		// Flip the 4-th bit
		return Piece(_piece ^ 8u);
	}

	constexpr operator u8() const noexcept
	{
		return static_cast<u8>(_piece);
	}

	static constexpr bool isValid(const PieceType type) noexcept
	{
		return PAWN <= type && type <= KING;
	}

private:
	/*
	 * The first 3 bits are use to store the specific Piece Type, eg. PAWN, KNIGHT, BISHOP
	 * The 4-th bit is use to indicate the color of this piece
	 */
	u8 _piece{};
	
};

constexpr Piece EMPTY_PIECE{};
