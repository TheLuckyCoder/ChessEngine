#pragma once

#include <array>
#include <cassert>

#include "Defs.h"

#ifdef _MSC_VER
#  include <intrin.h> // Microsoft header for _BitScanForward64()
#endif

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
	constexpr std::array<U64, 64> shiftedBoards = []
	{
		std::array<U64, 64> array{};

		for (auto i = 0u; i < 64u; ++i)
			array[i] = 1ULL << i;

		return array;
	}();

#if defined(__GNUC__)  // GCC, Clang

	inline byte bitScanForward(const U64 bb) noexcept
	{
		assert(bb);
		return static_cast<byte>(__builtin_ctzll(bb));
	}

	inline byte bitScanReverse(U64 bb) noexcept
	{
		assert(bb);
		return static_cast<byte>(63 ^ __builtin_clzll(bb));
	}

#elif defined(_MSC_VER)

	inline byte bitScanForward(const U64 bb) noexcept
	{
		assert(bb);
		unsigned long index{};

#	ifdef _WIN64 // 64-bit
		_BitScanForward64(&index, bb);
		return static_cast<byte>(index);
#	else // 32-bit
		if (bb & 0xffffffff) // If the bit is in the lower 32 bits
		{
			_BitScanForward(&index, static_cast<unsigned long>(bb));
			return static_cast<byte>(index);
		}

		_BitScanForward(&index, static_cast<unsigned long>(bb >> 32));
		return static_cast<byte>(index + 32u); // The bit is in the upper 32 bits
#	endif
	}

	inline byte bitScanReverse(const U64 bb) noexcept
	{
		assert(bb);
		unsigned long index{};

#	ifdef _WIN64 // 64-bit
		_BitScanReverse64(&index, bb);
		return static_cast<byte>(index);
#	else // 32-bit
		if (bb >> 32u) // If the bit is in the upper 32 bits
		{
			_BitScanReverse(&index, static_cast<unsigned long>(bb >> 32u));
			return static_cast<byte>(index + 32u);
		}

		_BitScanReverse(&index, static_cast<unsigned long>(bb));
		return static_cast<byte>(index);
#	endif
	}

#else

	/**
	 * @author Kim Walisch (2012)
	 * @param bb bitboard to scan
	 * @precondition bb != 0
	 * @return index (0..63) of least significant one bit
	 */
	inline byte bitScanForward(const U64 bb) noexcept
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
	 * @authors Kim Walisch, Mark Dickinson
	 * @param bb bitboard to scan
	 * @precondition bb != 0
	 * @return index (0..63) of most significant one bit
	 */
	inline byte bitScanReverse(U64 bb) noexcept
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

		bb |= bb >> 1;
		bb |= bb >> 2;
		bb |= bb >> 4;
		bb |= bb >> 8;
		bb |= bb >> 16;
		bb |= bb >> 32;

		return index64[(bb * debruijn64) >> 58];
	}

#endif

	inline byte popLsb(U64 &bb) noexcept
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
	inline int popCount(U64 x) noexcept
	{
		int count = 0;

		while (x)
		{
			x &= x - 1; // clear the least significant bit set
			++count;
		}

		return count;
	}

	inline byte findNextSquare(U64 &bb)
	{
		const byte square = Bitboard::bitScanForward(bb);
		bb ^= Bitboard::shiftedBoards[square];
		return square;
	}
}
