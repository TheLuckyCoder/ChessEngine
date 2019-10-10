#pragma once

#include <array>
#include <cassert>
#include <cstdint>

using byte = unsigned char;
using U64 = std::uint64_t;

constexpr U64 RANK_1 = 0xffull;
constexpr U64 RANK_2 = 0xff00ull;
constexpr U64 RANK_3 = 0xff0000ull;
constexpr U64 RANK_4 = 0xff000000ull;
constexpr U64 RANK_5 = 0xff00000000ull;
constexpr U64 RANK_6 = 0xff0000000000ull;
constexpr U64 RANK_7 = 0xff000000000000ull;
constexpr U64 RANK_8 = 0xff00000000000000ull;

constexpr U64 FILE_H = 0x8080808080808080ull;
constexpr U64 FILE_G = 0x4040404040404040ull;
constexpr U64 FILE_F = 0x2020202020202020ull;
constexpr U64 FILE_E = 0x1010101010101010ull;
constexpr U64 FILE_D = 0x808080808080808ull;
constexpr U64 FILE_C = 0x404040404040404ull;
constexpr U64 FILE_B = 0x202020202020202ull;
constexpr U64 FILE_A = 0x101010101010101ull;

constexpr byte row(const U64 pos) { return static_cast<byte>(pos / 8); }

constexpr byte col(const U64 pos) { return static_cast<byte>(pos % 8); }

namespace Bitboard
{
	constexpr U64 eastN(U64 board, const int n) {
		for (int i = 0; i < n; i++)
			board = ((board << 1) & (~FILE_A));

		return board;
	}

	constexpr U64 westN(U64 board, const int n) {
		for (int i = 0; i < n; i++)
			board = ((board >> 1) & (~FILE_H));

		return board;
	}

	constexpr std::array<U64, 64> indexedPos = [] {
        std::array<U64, 64> array{};
        array[0] = 1ull;

        for (auto i = 1u; i < 64u; ++i)
            array[i] = array[i - 1] << 1;

        return array;
    }();

	/**
	 * bitScanForward
	 * @author Martin Läuter (1997)
	 *         Charles E. Leiserson
	 *         Harald Prokop
	 *         Keith H. Randall
	 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
	 * @param bb bitboard to scan
	 * @precondition bb != 0
	 * @return index (0..63) of least significant one bit
	 */
	constexpr int bitScanForward(const U64 bb)
	{
        assert(bb != 0ull);

        constexpr int index64[64] = {
			0,  1, 48,  2, 57, 49, 28,  3,
			61, 58, 50, 42, 38, 29, 17,  4,
			62, 55, 59, 36, 53, 51, 43, 22,
			45, 39, 33, 30, 24, 18, 12,  5,
			63, 47, 56, 27, 60, 41, 37, 16,
			54, 35, 52, 21, 44, 32, 23, 11,
			46, 26, 40, 15, 34, 20, 31, 10,
			25, 14, 19,  9, 13,  8,  7,  6
		};

		constexpr U64 debruijn64{ 0x03f79d71b4cb0a89 };
		return index64[((bb & -static_cast<long long>(bb)) * debruijn64) >> 58];
	}

	/**
	 * popCount
	 * @author Brian Kernighan
	 * @param x bitboard to count the bits from
	 * @return number of 1 bits in the bitboard
	 */
    constexpr int popCount(U64 x)
    {
        int count = 0;

		while (x)
		{
			x &= x - 1; // clear the least significant bit set
			++count;
		}

        return count;
    }
}
