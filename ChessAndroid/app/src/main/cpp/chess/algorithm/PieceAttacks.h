#pragma once

#include <array>

#include "../data/Bitboard.h"

using byte = unsigned char;

class PieceAttacks
{
	const static std::array<U64, 64> s_KnightMoves;
	const static std::array<U64, 64> s_KingMoves;

public:
	PieceAttacks() = delete;
	PieceAttacks(const PieceAttacks&) = delete;
	PieceAttacks(PieceAttacks&&) = delete;

	static void init() noexcept;
	static U64 getKnightAttacks(byte square) noexcept;
	static U64 getKingAttacks(byte square) noexcept;
};
