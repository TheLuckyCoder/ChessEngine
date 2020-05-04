#pragma once

#include "../data/Bits.h"

class PieceAttacks
{
	static std::array<std::array<U64, 1024>, 64> _bishopAttacks;
	static std::array<std::array<U64, 4096>, 64> _rookAttacks;

public:
	PieceAttacks() = delete;
	PieceAttacks(const PieceAttacks&) = delete;
	PieceAttacks(PieceAttacks&&) = delete;
	~PieceAttacks() = delete;

	PieceAttacks &operator=(const PieceAttacks&) = delete;
	PieceAttacks &operator=(PieceAttacks&&) = delete;

	static void init() noexcept;
	static U64 knightAttacks(byte square) noexcept;
	static U64 bishopAttacks(byte square, U64 blockers) noexcept;
	static U64 rookAttacks(byte square, U64 blockers) noexcept;
	static U64 queenAttacks(byte square, U64 blockers) noexcept;
	static U64 kingAttacks(byte square) noexcept;

	template <Color C>
	static U64 pawnAttacks(const U64 pawns) noexcept
	{
		static_assert(C == WHITE || C == BLACK);
		
		if constexpr (C == WHITE)
			return Bits::shift<NORTH_WEST>(pawns) | Bits::shift<NORTH_EAST>(pawns);
		else
			return Bits::shift<SOUTH_WEST>(pawns) | Bits::shift<SOUTH_EAST>(pawns);
	}

	template <Color C>
	static U64 pawnDoubleAttacks(const U64 pawns) noexcept
	{
		static_assert(C == WHITE || C == BLACK);
		
		if constexpr (C == WHITE)
			return Bits::shift<NORTH_WEST>(pawns) & Bits::shift<NORTH_EAST>(pawns);
		else
			return Bits::shift<SOUTH_WEST>(pawns) & Bits::shift<SOUTH_EAST>(pawns);
	}
};
