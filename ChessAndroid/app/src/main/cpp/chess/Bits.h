#pragma once

#include <array>
#include <cassert>

#include "Defs.h"

#ifdef _MSC_VER
#  include <intrin.h> // Microsoft header for _BitScanForward64()
#elif defined(__linux__)
#	include <byteswap.h>
#endif

//#define USE_CUSTOM_POPCNT

namespace Bits
{
	static constexpr U64 _eastN(U64 board, const int n) noexcept
	{
		for (int i = 0; i < n; ++i)
			board = ((board << 1) & (~FILE_A));

		return board;
	}

	static constexpr U64 _westN(U64 board, const int n) noexcept
	{
		for (int i = 0; i < n; ++i)
			board = ((board >> 1) & (~FILE_H));

		return board;
	}

	/**
	 * Table of precalculated shifted bitboards indexed by the times 1 has been shifted to the left
	 */
	static constexpr auto _SQUARES = []
	{
		std::array<U64, 64> array{};

		for (byte i = 0u; i < SQUARE_NB; ++i)
			array[i] = 1ull << i;

		return array;
	}();

	constexpr U64 getSquare64(const byte square) noexcept
	{
		return _SQUARES[square];
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

#elif defined(__linux__)

	inline U64 flipVertical(const U64 bb)
	{
		return bswap_64(bb);
	}

#elif (defined(__clang__) && __has_builtin(__builtin_bswap64)) \
  || (defined(__GNUC__ ) && (__GNUC__ > 4))

	inline U64 flipVertical(const U64 bb)
	{
		return __builtin_bswap64(bb);
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

	inline bool several(const U64 bb) noexcept
	{
		return bb & (bb - 1);
	}

	inline bool onlyOne(const U64 bb) noexcept
	{
		return bb && !several(bb);
	}

// region Rays
	/**
	* Table of precalculated ray bitboards indexed by direction and square
	*/
	static constexpr auto _RAYS = []
	{
		std::array<std::array<U64, SQUARE_NB>, 8> rays{};

		using namespace Bits;

		for (byte square{}; square < SQUARE_NB; ++square)
		{
			rays[NORTH][square] = 0x0101010101010100ULL << square;

			rays[SOUTH][square] = 0x0080808080808080ULL >> (63u - square);

			rays[EAST][square] = 2u * (_SQUARES[square | 7u] - _SQUARES[square]);

			rays[WEST][square] = _SQUARES[square] - _SQUARES[square & 56u];

			rays[NORTH_WEST][square] =
				_westN(0x102040810204000ULL, 7u - col(square)) << (row(square) * 8u);

			rays[NORTH_EAST][square] =
				_eastN(0x8040201008040200ULL, col(square)) << (row(square) * 8u);

			rays[SOUTH_WEST][square] =
				_westN(0x40201008040201ULL, 7u - col(square)) >> ((7u - row(square)) * 8u);

			rays[SOUTH_EAST][square] =
				_eastN(0x2040810204080ULL, col(square)) >> ((7u - row(square)) * 8u);
		}

		return rays;
	}();

	static constexpr auto _RANKS = []
	{
		std::array<U64, 8> ranks{};

		for (byte r = 0u; r < 8u; ++r)
			ranks[r] = 0b1111'1111ull << (8u * r);

		return ranks;
	}();

	static constexpr auto _FILES = []
	{
		std::array<U64, 8> files{};

		for (byte f = 0u; f < 8u; ++f)
			files[f] = 0x101010101010101ull << f;

		return files;
	}();

	template <Dir D>
	static constexpr U64 _shiftT(const U64 bb) noexcept
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

	template <Dir D, Dir... Dirs>
	constexpr U64 shift(const U64 bb) noexcept
	{
		U64 result = _shiftT<D>(bb);
		((result = _shiftT<Dirs>(result)),...);
		return result;
	}

	/**
	 * Returns a bitboard containing the given ray in the given direction.
	 *
	 * @param direction Direction of ray to return
	 * @param square Square to get ray starting from (in little endian rank file mapping form)
	 */
	constexpr U64 getRay(const Dir direction, const byte square) noexcept
	{
		return _RAYS[direction][square];
	}

	constexpr U64 getRank(const byte square) noexcept
	{
		return _RANKS[row(square)];
	}

	constexpr U64 getFile(const byte square) noexcept
	{
		return _FILES[col(square)];
	}

	constexpr U64 getAdjacentFiles(const byte square) noexcept
	{
		return shift<WEST>(getFile(square)) | shift<EAST>(getFile(square));
	}

	inline static U64 _generateRayAttacksForwards(const byte sq, const U64 occupied, const Dir direction)
	{
		const U64 attacks = getRay(direction, sq);
		const U64 blocker = attacks & occupied;
		const byte index = bitScanForward(blocker | 0x8000000000000000);
		return attacks ^ getRay(direction, index);
	}

	inline static U64 _generateRayAttacksBackwards(const byte sq, const U64 occupied, const Dir direction)
	{
		const U64 attacks = Bits::getRay(direction, sq);
		const U64 blocker = attacks & occupied;
		const byte index = bitScanReverse(blocker | 1ULL);
		return attacks ^ getRay(direction, index);
	}

	inline U64 generateBishopAttacks(const byte square, const U64 blockers) noexcept
	{
		return _generateRayAttacksForwards(square, blockers, NORTH_WEST)
			   | _generateRayAttacksForwards(square, blockers, NORTH_EAST)
			   | _generateRayAttacksBackwards(square, blockers, SOUTH_EAST)
			   | _generateRayAttacksBackwards(square, blockers, SOUTH_WEST);
	}

	inline U64 generateRookAttacks(const byte square, const U64 blockers) noexcept
	{
		return _generateRayAttacksForwards(square, blockers, NORTH)
			   | _generateRayAttacksForwards(square, blockers, EAST)
			   | _generateRayAttacksBackwards(square, blockers, SOUTH)
			   | _generateRayAttacksBackwards(square, blockers, WEST);
	}

	static const auto _RAYS_BETWEEN_SQUARES = []
	{
		std::array<std::array<U64, SQUARE_NB>, SQUARE_NB> array{};

		for (byte sq1{}; sq1 < SQUARE_NB; ++sq1)
			for (byte sq2{}; sq2 < SQUARE_NB; ++sq2)
			{
				const U64 bb1 = getSquare64(sq1);
				const U64 bb2 = getSquare64(sq2);

				if (generateRookAttacks(sq1, 0ull) & bb2)
					array[sq1][sq2] = generateRookAttacks(sq1, bb2) & generateRookAttacks(sq2, bb1);
				else if (generateBishopAttacks(sq1, 0ull) & bb2)
					array[sq1][sq2] = generateBishopAttacks(sq1, bb2) & generateBishopAttacks(sq2, bb1);
			}

		return array;
	}();

	inline U64 getRayBetween(const byte sq1, const byte sq2)
	{
		assert(sq1 < SQ_NONE);
		assert(sq2 < SQ_NONE);
		return _RAYS_BETWEEN_SQUARES[sq1][sq2];
	}

// endregion Rays

	/**
	 * Calculated using Chebyshev distance
	 */
	static constexpr auto _DISTANCE_BETWEEM_SQUARES = []
	{
		std::array<std::array<byte, SQUARE_NB>, SQUARE_NB> array{};

		constexpr auto abs = [] (const int x)
		{
			return x < 0 ? -x : x;
		};

		for (byte x{}; x < SQUARE_NB; ++x)
			for (byte y{}; y < SQUARE_NB; ++y)
				array[x][y] = std::max<byte>(abs(row(x) - row(y)), abs(col(x) - col(y)));

		return array;
	}();

	constexpr byte getDistanceBetween(const byte sq1, const byte sq2)
	{
		return _DISTANCE_BETWEEM_SQUARES[sq1][sq2];
	}
}

#undef USE_CUSTOM_POPCNT
