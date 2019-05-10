#pragma once

#include "../data/Bitboard.h"

using byte = unsigned char;

class PieceAttacks
{
public:
	PieceAttacks() = delete;
	PieceAttacks(const PieceAttacks&) = delete;
	PieceAttacks(PieceAttacks&&) = delete;

	static void init() noexcept;
	static U64 getKnightAttacks(byte square) noexcept;
	static U64 getKingAttacks(byte square) noexcept;
};
