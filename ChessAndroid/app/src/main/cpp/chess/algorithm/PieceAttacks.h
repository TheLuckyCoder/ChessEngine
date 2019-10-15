#pragma once

#include <array>

#include "../data/Bitboard.h"

using byte = unsigned char;

class PieceAttacks
{
	const static std::array<U64, 64> s_PawnAttacks;
	const static std::array<U64, 64> s_KnightAttacks;
	const static std::array<U64, 64> s_KingAttacks;

public:
	PieceAttacks() = delete;
	PieceAttacks(const PieceAttacks&) = delete;
	PieceAttacks(PieceAttacks&&) = delete;

	static U64 getKnightAttacks(byte square) noexcept;
	static U64 getBishopAttacks(byte square, U64 blockers) noexcept;
	static U64 getRookAttacks(byte square, U64 blockers) noexcept;
	static U64 getKingAttacks(byte square) noexcept;
};
