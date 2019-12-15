#pragma once

#include "Pos.h"

class Board;

class Piece final
{
public:
	enum Type : byte
	{
		NO_PIECE,
		W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
		B_PAWN = 9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
		PIECE_NB = 16
	};

private:
	/*
	 * The first 3 bits are use to store the specific Piece Type, eg. PAWN, KNIGHT, BISHOP
	 * The 4-th bit is use to indicate the color of this piece
	 */
	Type content;
public:
	const static Piece EMPTY;

	constexpr Piece() noexcept
		: content(NO_PIECE) {}
	
	constexpr Piece(const PieceType type, const Color color) noexcept
		: content(static_cast<Type>((color << 3u) | type)) {}

	explicit constexpr Piece(const Type type) noexcept
		: content(type) {}
	
	Piece(Piece&&) = default;
	Piece(const Piece&) = default;
	~Piece() = default;

	Piece &operator=(const Piece &other) = default;
	Piece &operator=(Piece &&other) = default;

	constexpr bool operator==(const Piece &other) const noexcept
	{
		return content == other.content;
	}

	constexpr bool operator==(const Type type) const noexcept
	{
		return content == type;
	}

	constexpr Color color() const noexcept
	{
		return static_cast<Color>(content >> 3u);
	}

	constexpr PieceType type() const noexcept
	{
		return static_cast<PieceType>(content & 7u);
	}

	constexpr bool isSameColor(const Piece &other) const noexcept
	{
		return color() == other.color();
	}

	constexpr Piece operator~() const noexcept
	{
		// Flip the 4-th bit
		return Piece(static_cast<Type>(content ^ 8u));
	}

	constexpr operator byte() const noexcept
	{
		return static_cast<byte>(content);
	}

	constexpr operator bool() const noexcept
	{
		return content != Type::NO_PIECE;
	}

	U64 getPossibleMoves(byte square, const Board &board) const noexcept;
	U64 getPossibleCaptures(byte square, const Board &board) const noexcept;
};
