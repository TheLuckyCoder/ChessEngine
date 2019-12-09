#pragma once

#include "Bitboard.h"

namespace Rays
{

	enum Dir
	{
		NORTH,
		SOUTH,
		EAST,
		WEST,
		NORTH_EAST,
		NORTH_WEST,
		SOUTH_EAST,
		SOUTH_WEST
	};

	/**
	 * Table of precalculated ray bitboards indexed by direction and square
	 */
	constexpr static std::array<std::array<U64, 64>, 8> rays = [] {
		std::array<std::array<U64, 64>, 8> rays{};

		using namespace Bitboard;

		for (byte square = 0u; square < 64u; square++)
		{
			rays[NORTH][square] = 0x0101010101010100ULL << square;

			rays[SOUTH][square] = 0x0080808080808080ULL >> (63u - square);

			rays[EAST][square] = 2u * (shiftedBoards[square | 7u] - shiftedBoards[square]);

			rays[WEST][square] = shiftedBoards[square] - shiftedBoards[square & 56u];

			rays[NORTH_WEST][square] = westN(0x102040810204000ULL, 7u - col(square)) << (row(square) * 8u);

			rays[NORTH_EAST][square] = eastN(0x8040201008040200ULL, col(square)) << (row(square) * 8u);

			rays[SOUTH_WEST][square] = westN(0x40201008040201ULL, 7u - col(square)) >> ((7u - row(square)) * 8u);

			rays[SOUTH_EAST][square] = eastN(0x2040810204080ULL, col(square)) >> ((7u - row(square)) * 8u);
		}

		return rays;
	}();

	/**
	 * Returns a bitboard containing the given ray in the given direction.
	 *
	 * @param direction Direction of ray to return
	 * @param square Square to get ray starting from (in little endian rank file mapping form)
	 */
	constexpr U64 getRay(const Dir direction, const byte square) noexcept
	{
		return rays[direction][square];
	}
};
