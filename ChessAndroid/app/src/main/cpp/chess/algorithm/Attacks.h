#pragma once

#include "../Bits.h"

class Attacks
{
	static std::array<std::array<u64, 1024>, 64> _bishopAttacks;
	static std::array<std::array<u64, 4096>, 64> _rookAttacks;

public:
	Attacks() = delete;
	Attacks(const Attacks&) = delete;
	Attacks(Attacks&&) = delete;
	~Attacks() = delete;

	Attacks &operator=(const Attacks&) = delete;
	Attacks &operator=(Attacks&&) = delete;

	static void init() noexcept;

	static u64 knightAttacks(u8 square) noexcept;
	static u64 bishopAttacks(u8 square, u64 blockers) noexcept;
	static u64 rookAttacks(u8 square, u64 blockers) noexcept;
	static u64 queenAttacks(u8 square, u64 blockers) noexcept;
	static u64 kingAttacks(u8 square) noexcept;

	static u64 bishopXRayAttacks(u8 square) noexcept;
	static u64 rookXRayAttacks(u8 square) noexcept;

	template <Color C>
	static u64 pawnAttacks(const u64 pawns) noexcept
	{
		static_assert(C == WHITE || C == BLACK);

		if constexpr (C == WHITE)
			return Bits::shift<NORTH_WEST>(pawns) | Bits::shift<NORTH_EAST>(pawns);
		else
			return Bits::shift<SOUTH_WEST>(pawns) | Bits::shift<SOUTH_EAST>(pawns);
	}

	template <Color C>
	static u64 pawnDoubleAttacks(const u64 pawns) noexcept
	{
		static_assert(C == WHITE || C == BLACK);

		if constexpr (C == WHITE)
			return Bits::shift<NORTH_WEST>(pawns) & Bits::shift<NORTH_EAST>(pawns);
		else
			return Bits::shift<SOUTH_WEST>(pawns) & Bits::shift<SOUTH_EAST>(pawns);
	}
};
