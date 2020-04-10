#pragma once

#include "Defs.h"

class Piece final
{
public:
	Piece() noexcept = default;
	
	constexpr Piece(const PieceType type, const Color color) noexcept
		: _piece((color << 3u) | type) {}

	explicit constexpr Piece(const byte type) noexcept
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

	constexpr bool operator==(const byte type) const noexcept
	{
		return _piece == type;
	}

	constexpr Color color() const noexcept
	{
		return static_cast<Color>(_piece >> 3u);
	}

	constexpr PieceType type() const noexcept
	{
		return static_cast<PieceType>(_piece & 7u);
	}

	constexpr bool isSameColor(const Piece &other) const noexcept
	{
		return color() == other.color();
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

	constexpr operator byte() const noexcept
	{
		return static_cast<byte>(_piece);
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
	byte _piece{};
	
};

constexpr Piece EMPTY_PIECE{};
