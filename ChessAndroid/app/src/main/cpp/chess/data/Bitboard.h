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

constexpr byte row(const byte pos) noexcept { return static_cast<byte>(pos / 8u); }

constexpr byte col(const byte pos) noexcept { return static_cast<byte>(pos % 8u); }

namespace Bitboard
{
	constexpr U64 eastN(U64 board, const int n) noexcept
	{
		for (int i = 0; i < n; ++i)
			board = ((board << 1) & (~FILE_A));

		return board;
	}

	constexpr U64 westN(U64 board, const int n) noexcept
	{
		for (int i = 0; i < n; ++i)
			board = ((board >> 1) & (~FILE_H));

		return board;
	}

	/**
	 * Table of precalculated shifted bitboards indexed by the times 1 has been shifted to the left
	 */
	constexpr std::array<U64, 64> shiftedBoards = [] {
        std::array<U64, 64> array{};

        for (auto i = 0u; i < 64u; ++i)
            array[i] = 1ULL << i;

        return array;
    }();

	/**
	 * bitScanForward
	 * @author Kim Walisch (2012)
	 * @param bb bitboard to scan
	 * @precondition bb != 0
	 * @return index (0..63) of least significant one bit
	 */
	constexpr byte bitScanForward(const U64 bb) noexcept
	{
		assert(bb);

		constexpr U64 debruijn64{ 0x03f79d71b4cb0a89 };
		constexpr byte index64[64] = {
			0, 47,  1, 56, 48, 27,  2, 60,
			57, 49, 41, 37, 28, 16,  3, 61,
			54, 58, 35, 52, 50, 42, 21, 44,
			38, 32, 29, 23, 17, 11,  4, 62,
			46, 55, 26, 59, 40, 36, 15, 53,
			34, 51, 20, 43, 31, 22, 10, 45,
			25, 39, 14, 33, 19, 30,  9, 24,
			13, 18,  8, 12,  7,  6,  5, 63
		};

		return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
	}

	/**
	 * bitScanReverse
	 * @authors Kim Walisch, Mark Dickinson
	 * @param bb bitboard to scan
	 * @precondition bb != 0
	 * @return index (0..63) of most significant one bit
	 */
	constexpr byte bitScanReverse(U64 bb) noexcept
	{
		assert (bb != 0);

		constexpr U64 debruijn64{ 0x03f79d71b4cb0a89 };
		constexpr byte index64[64] = {
			0, 47,  1, 56, 48, 27,  2, 60,
			57, 49, 41, 37, 28, 16,  3, 61,
			54, 58, 35, 52, 50, 42, 21, 44,
			38, 32, 29, 23, 17, 11,  4, 62,
			46, 55, 26, 59, 40, 36, 15, 53,
			34, 51, 20, 43, 31, 22, 10, 45,
			25, 39, 14, 33, 19, 30,  9, 24,
			13, 18,  8, 12,  7,  6,  5, 63
		};

		bb |= bb >> 1;
		bb |= bb >> 2;
		bb |= bb >> 4;
		bb |= bb >> 8;
		bb |= bb >> 16;
		bb |= bb >> 32;

		return index64[(bb * debruijn64) >> 58];
	}

	constexpr byte popLsb(U64 &bb) noexcept
	{
		const byte lsbIndex = bitScanForward(bb);
		bb &= bb - 1;
		return lsbIndex;
	}

	/**
	 * popCount
	 * @author Brian Kernighan
	 * @param x bitboard to count the bits from
	 * @return number of 1 bits in the bitboard
	 */
    constexpr int popCount(U64 x) noexcept
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
