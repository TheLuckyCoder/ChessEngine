#pragma once

#include <array>
#include <cassert>

#include "Defs.h"

#ifdef _MSC_VER
#  include <intrin.h> // Microsoft header for _BitScanForward64()
#endif

//#define USE_CUSTOM_POPCNT

namespace Bits
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
	constexpr auto shiftedBoards = []
	{
		std::array<U64, 64> array{};

		for (auto i = 0u; i < SQUARE_NB; ++i)
			array[i] = 1ULL << i;

		return array;
	}();

	constexpr U64 getSquare64(const byte square) noexcept
	{
		return shiftedBoards[square];
	}

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

#else // No specific compiler found, fallback to general functions
#	undef USE_CUSTOM_POPCNT
#	define USE_CUSTOM_POPCNT

	static constexpr U64 debruijn64{ 0x03f79d71b4cb0a89 };
	static constexpr byte bitScanIndex64[64] = {
		0, 47,  1, 56, 48, 27,  2, 60,
		57, 49, 41, 37, 28, 16,  3, 61,
		54, 58, 35, 52, 50, 42, 21, 44,
		38, 32, 29, 23, 17, 11,  4, 62,
		46, 55, 26, 59, 40, 36, 15, 53,
		34, 51, 20, 43, 31, 22, 10, 45,
		25, 39, 14, 33, 19, 30,  9, 24,
		13, 18,  8, 12,  7,  6,  5, 63
	};

	/**
	 * @author Kim Walisch (2012)
	 * @param bb bitboard to scan
	 * @precondition bb != 0
	 * @return index (0..63) of least significant one bit
	 */
	inline byte bitScanForward(const U64 bb) noexcept
	{
		assert(bb);
		return bitScanIndex64[((bb ^ (bb - 1)) * debruijn64) >> 58];
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

		bb |= bb >> 1;
		bb |= bb >> 2;
		bb |= bb >> 4;
		bb |= bb >> 8;
		bb |= bb >> 16;
		bb |= bb >> 32;

		return bitScanIndex64[(bb * debruijn64) >> 58];
	}

#endif

#if defined(USE_CUSTOM_POPCNT)
	/**
	 * @author Brian Kernighan
	 * @param x bitboard to count the bits from
	 * @return number of 1 bits in the bitboard
	 */
	inline int popCount(U64 bb) noexcept
	{
		int count = 0;

		while (bb)
		{
			bb &= bb - 1; // clear the least significant bit set
			++count;
		}

		return count;
	}
#	elif defined(__GNUC__)  // GCC, Clang

	inline int popCount(const U64 bb) noexcept
	{
		return __builtin_popcountll(bb);
	}

#	elif defined(_MSC_VER)

	inline int popCount(const U64 bb) noexcept
	{
#	ifdef _WIN64 // 64-bit
		return static_cast<int>(__popcnt64(bb));
#	else // 32-bit
		return static_cast<int>(__popcnt(unsigned(bb)) + __popcnt(unsigned(bb >> 32u)));
#	endif
	}

#	endif

#if defined(_MSC_VER) && defined(_WIN64)

	inline U64 flipVertical(const U64 bb)
	{
		return _byteswap_uint64(bb);
	}
	
#else
	/**
	 * Flip a bitboard vertically about the centre ranks.
	 * Rank 1 is mapped to rank 8 and vice versa.
	 * @param bb any bitboard
	 * @return bitboard bb flipped vertically
	 */
	inline U64 flipVertical(U64 bb)
	{
		const U64 k1{ 0x00FF00FF00FF00FF };
		const U64 k2{ 0x0000FFFF0000FFFF };
		bb = ((bb >>  8) & k1) | ((bb & k1) <<  8);
		bb = ((bb >> 16) & k2) | ((bb & k2) << 16);
		bb = ( bb >> 32)       | ( bb       << 32);
		return bb;
	}
#endif

	inline byte popLsb(U64 &bb) noexcept
	{
		const byte lsbIndex = bitScanForward(bb);
		bb &= bb - 1;
		return lsbIndex;
	}

	inline byte findNextSquare(U64 &bb)
	{
		const byte square = Bits::bitScanForward(bb);
		bb ^= Bits::shiftedBoards[square];
		return square;
	}

// region Rays
	/**
	* Table of precalculated ray bitboards indexed by direction and square
	*/
	static constexpr auto RAYS = []
	{
		std::array<std::array<U64, SQUARE_NB>, 8> rays{};

		using namespace Bits;

		for (byte square = 0u; square < SQUARE_NB; ++square)
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

	static constexpr auto RANKS = []
	{
		std::array<U64, 8> ranks{};

		for (byte r = 0u; r < 8u; ++r)
			ranks[r] = 0b1111'1111ull << (8u * r);

		return ranks;
	}();

	static constexpr auto FILES = []
	{
		std::array<U64, 8> files{};

		for (byte f = 0u; f < 8u; ++f)
			files[f] = 0x101010101010101ull << f;

		return files;
	}();

	template <Dir D>
	constexpr U64 shift(const U64 bb) noexcept
	{
		if constexpr (D == NORTH)
			return bb << 8u;
		else if constexpr (D == SOUTH)
			return bb >> 8u;
		else if constexpr (D == EAST)
			return (bb & ~FILE_H) << 1u;
		else if constexpr (D == WEST)
			return (bb & ~FILE_A) >> 1u;

		else if constexpr (D == NORTH_EAST)
			return (bb & ~FILE_H) << 9u;
		else if constexpr (D == NORTH_WEST)
			return (bb & ~FILE_A) << 7u;
		else if constexpr (D == SOUTH_EAST)
			return (bb & ~FILE_H) >> 7u;
		else if constexpr (D == SOUTH_WEST)
			return (bb & ~FILE_A) >> 9u;

		return {};
	}

	/**
	 * Returns a bitboard containing the given ray in the given direction.
	 *
	 * @param direction Direction of ray to return
	 * @param square Square to get ray starting from (in little endian rank file mapping form)
	 */
	constexpr U64 getRay(const Dir direction, const byte square) noexcept
	{
		return RAYS[direction][square];
	}

	constexpr U64 getRank(const byte square) noexcept
	{
		return RANKS[row(square)];
	}

	constexpr U64 getFile(const byte square) noexcept
	{
		return FILES[col(square)];
	}

	constexpr U64 getAdjacentFiles(const byte square) noexcept
	{
		return shift<WEST>(getFile(square)) | shift<EAST>(getFile(square));
	}

	/**
	 * Calculated using Chebyshev distance
	 */
	static constexpr auto SQUARE_DISTANCE = []
	{
		constexpr auto abs = [](const int x)
		{
			return( // deal with signed-zeros
				x == 0 ? 0 :
				// else
				x < 0 ? - x : x );
		};

		std::array<std::array<byte, SQUARE_NB>, SQUARE_NB> array{};

		for (byte x{}; x < SQUARE_NB; ++x)
			for (byte y{}; y < SQUARE_NB; ++y)
				array[x][y] = std::max<byte>(abs(row(x) - row(y)), abs(col(x) - col(y)));

		return array;
	}();

	constexpr byte getDistance(const byte x, const byte y)
	{
		return SQUARE_DISTANCE[x][y];
	}

// endregion Rays
}

#undef USE_CUSTOM_POPCNT
