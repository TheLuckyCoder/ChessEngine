#pragma once

#include <array>

#include "Piece.h"

class Board;

class Zobrist final
{
public:
	Zobrist() = delete;
	Zobrist(const Zobrist &) = delete;
	Zobrist(Zobrist &&) = delete;
	~Zobrist() = delete;

	Zobrist &operator=(const Zobrist &) = delete;
	Zobrist &operator=(Zobrist &&) = delete;

	static u64 compute(const Board &board) noexcept;

	static void xorPiece(u64 &key, Square square, Piece piece) noexcept;
	static void flipSide(u64 &key) noexcept;
	static void xorCastlingRights(u64 &key, CastlingRights rights) noexcept;
	static void xorEnPassant(u64 &key, Square square) noexcept;
};
