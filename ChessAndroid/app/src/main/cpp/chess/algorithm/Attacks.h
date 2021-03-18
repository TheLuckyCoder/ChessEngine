#pragma once

#include "../Bits.h"

class Attacks
{
public:
	Attacks() = delete;
	Attacks(const Attacks &) = delete;
	Attacks(Attacks &&) = delete;
	~Attacks() = delete;

	Attacks &operator=(const Attacks &) = delete;
	Attacks &operator=(Attacks &&) = delete;

	static void init();

	static Bitboard knightAttacks(Square square) noexcept;
	static Bitboard bishopAttacks(Square square, Bitboard blockers) noexcept;
	static Bitboard rookAttacks(Square square, Bitboard blockers) noexcept;
	static Bitboard queenAttacks(Square square, Bitboard blockers) noexcept;
	static Bitboard kingAttacks(Square square) noexcept;

	static Bitboard bishopXRayAttacks(Square square) noexcept;
	static Bitboard rookXRayAttacks(Square square) noexcept;

	template <Color C>
	static constexpr Bitboard pawnAttacks(const Bitboard pawns) noexcept
	{
		static_assert(C == WHITE || C == BLACK);

		if constexpr (C == WHITE)
			return pawns.shift<NORTH_WEST>() | pawns.shift<NORTH_EAST>();
		else
			return pawns.shift<SOUTH_WEST>() | pawns.shift<SOUTH_EAST>();
	}

	template <Color C>
	static constexpr Bitboard pawnDoubleAttacks(const Bitboard pawns) noexcept
	{
		static_assert(C == WHITE || C == BLACK);

		if constexpr (C == WHITE)
			return pawns.shift<NORTH_WEST>() & pawns.shift<NORTH_EAST>();
		else
			return pawns.shift<SOUTH_WEST>() & pawns.shift<SOUTH_EAST>();
	}
};
