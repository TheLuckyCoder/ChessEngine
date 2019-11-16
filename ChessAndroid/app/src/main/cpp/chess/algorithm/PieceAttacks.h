#pragma once

#include <array>

#include "../data/Bitboard.h"

using byte = unsigned char;

class PieceAttacks
{
	const static std::array<std::array<U64, 64>, 2> s_PawnAttacks;
	const static std::array<U64, 64> s_KnightAttacks;
	static std::array<std::array<U64, 1024>, 64> s_BishopAttacks;
	static std::array<std::array<U64, 4096>, 64> s_RookAttacks;
	const static std::array<U64, 64> s_KingAttacks;

public:
	PieceAttacks() = delete;
	PieceAttacks(const PieceAttacks&) = delete;
	PieceAttacks(PieceAttacks&&) = delete;

	static void init() noexcept;
	static U64 getPawnAttacks(bool isWhite, byte square) noexcept;
	static U64 getKnightAttacks(byte square) noexcept;
	static U64 getBishopAttacks(byte square, U64 blockers) noexcept;
	static U64 getRookAttacks(byte square, U64 blockers) noexcept;
	static U64 getKingAttacks(byte square) noexcept;
};
