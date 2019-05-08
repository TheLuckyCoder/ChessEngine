#pragma once

#include "../data/Bitboard.h"

using byte = unsigned char;

class PieceAttacks
{
	static U64 m_KnightMoves[64];
	static U64 m_KingMoves[64];

public:
	PieceAttacks() = delete;
	PieceAttacks(const PieceAttacks&) = delete;
	PieceAttacks(PieceAttacks&&) = delete;

	static void init() noexcept;

private:
	static void initKnightAttacks() noexcept;
	static void initKingAttacks() noexcept;

public:
	static U64 getKnightAttacks(byte square) noexcept;
	static U64 getKingAttacks(byte square) noexcept;
};
