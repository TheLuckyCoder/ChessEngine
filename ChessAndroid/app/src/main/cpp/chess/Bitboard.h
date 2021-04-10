#pragma once

#include <array>
#include <bit>

#include "Defs.h"

#ifdef _MSC_VER
#  include <intrin.h>
#elif defined(__linux__)

#	include <byteswap.h>

#endif

namespace Bits
{
	static constexpr u64 FILE_A_BB{ 0x101010101010101ull };
	static constexpr u64 FILE_H_BB{ 0x8080808080808080ull };

	static constexpr u64 _eastN(u64 board, const u8 n) noexcept
	{
		for (u8 i{}; i < n; ++i)
			board = ((board << 1) & (~FILE_A_BB));

		return board;
	}

	static constexpr u64 _westN(u64 board, const u8 n) noexcept
	{
		for (int i{}; i < n; ++i)
			board = ((board >> 1) & (~FILE_H_BB));

		return board;
	}

#if defined(_MSC_VER) && defined(_WIN64)

	inline u64 flipVertical(const u64 bb) noexcept
	{
		return _byteswap_uint64(bb);
	}

#elif defined(__linux__)

	inline u64 flipVertical(const u64 bb) noexcept
	{
		return bswap_64(bb);
	}

#elif defined(__has_builtin) && __has_builtin(__builtin_bswap64)

	inline u64 flipVertical(const u64 bb) noexcept
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
	inline u64 flipVertical(u64 bb) noexcept
	{
		constexpr u64 k1{ 0x00FF00FF00FF00FF };
		constexpr u64 k2{ 0x0000FFFF0000FFFF };
		bb = ((bb >>  8) & k1) | ((bb & k1) <<  8);
		bb = ((bb >> 16) & k2) | ((bb & k2) << 16);
		bb = ( bb >> 32)       | ( bb       << 32);
		return bb;
	}

#endif

	/**
	 * Calculated using Chebyshev distance
	 */
	static constexpr auto DistanceBetweenSquares = []
	{
		std::array<std::array<u8, SQUARE_NB>, SQUARE_NB> array{};

		constexpr auto abs = [](const int x)
		{
			return x < 0 ? -x : x;
		};

		for (u8 x{}; x < SQUARE_NB; ++x)
			for (u8 y{}; y < SQUARE_NB; ++y)
				array[x][y] = std::max<u8>(abs(rankOf(x) - rankOf(y)), abs(fileOf(x) - fileOf(y)));

		return array;
	}();

	/**
	 * Table of precalculated shifted bitboards indexed by the times 1 has been shifted to the left
	 */
	static constexpr auto Squares = []
	{
		std::array<u64, SQUARE_NB> array{};

		for (u8 i{}; i < SQUARE_NB; ++i)
			array[i] = 1ull << i;

		return array;
	}();

	// region Rays
	/**
	* Table of precalculated ray bitboards indexed by direction and square
	*/
	static constexpr auto Rays = []
	{
		std::array<std::array<u64, SQUARE_NB>, 8> rays{};

		for (u8 square{}; square < SQUARE_NB; ++square)
		{
			rays[NORTH][square] = 0x0101010101010100ULL << square;

			rays[SOUTH][square] = 0x0080808080808080ULL >> (63u - square);

			rays[EAST][square] = 2u * (Squares[square | 7u] - Squares[square]);

			rays[WEST][square] = Squares[square] - Squares[square & 56u];

			rays[NORTH_WEST][square] =
				_westN(0x102040810204000ULL, 7u - fileOf(square)) << (rankOf(square) * 8u);

			rays[NORTH_EAST][square] =
				_eastN(0x8040201008040200ULL, fileOf(square)) << (rankOf(square) * 8u);

			rays[SOUTH_WEST][square] =
				_westN(0x40201008040201ULL, 7u - fileOf(square)) >> ((7u - rankOf(square)) * 8u);

			rays[SOUTH_EAST][square] =
				_eastN(0x2040810204080ULL, fileOf(square)) >> ((7u - rankOf(square)) * 8u);
		}

		return rays;
	}();

	static constexpr auto Ranks = []
	{
		std::array<u64, 8> ranks{};

		for (u8 r{}; r < 8u; ++r)
			ranks[r] = 0b1111'1111ull << (8u * r);

		return ranks;
	}();

	static constexpr auto Files = []
	{
		std::array<u64, 8> files{};

		for (u8 f{}; f < 8u; ++f)
			files[f] = FILE_A_BB << f;

		return files;
	}();

	template <Dir D>
	static constexpr u64 _shiftT(const u64 bb) noexcept
	{
		if constexpr (D == NORTH)
			return bb << 8u;
		else if constexpr (D == SOUTH)
			return bb >> 8u;
		else if constexpr (D == EAST)
			return (bb & ~FILE_H_BB) << 1u;
		else if constexpr (D == WEST)
			return (bb & ~FILE_A_BB) >> 1u;

		else if constexpr (D == NORTH_EAST)
			return (bb & ~FILE_H_BB) << 9u;
		else if constexpr (D == NORTH_WEST)
			return (bb & ~FILE_A_BB) << 7u;
		else if constexpr (D == SOUTH_EAST)
			return (bb & ~FILE_H_BB) >> 7u;
		else if constexpr (D == SOUTH_WEST)
			return (bb & ~FILE_A_BB) >> 9u;

		return {};
	}

	constexpr u8 bitScanForward(const u64 x)
	{
		return u8(std::countr_zero(x));
	}

	constexpr u8 bitScanReverse(const u64 x)
	{
		return u8(63 ^ std::countl_zero(x));
	}

	static constexpr u64 _generateRayAttacksForwards(const u8 sq, const u64 occupied, const Dir direction)
	{
		const u64 attacks = Rays[direction][sq];
		const u64 blockers = attacks & occupied;
		const u8 square = bitScanForward(blockers | 0x8000000000000000);
		return attacks ^ Rays[direction][square];
	}

	static constexpr u64 _generateRayAttacksBackwards(const u8 sq, const u64 occupied, const Dir direction)
	{
		const auto attacks = Rays[direction][sq];
		const auto blockers = attacks & occupied;
		const u8 square = bitScanReverse(blockers | 1ull);
		return attacks ^ Rays[direction][square];
	}

	static constexpr u64 generateBishopAttacks(const u8 square, const u64 blockers) noexcept
	{
		return _generateRayAttacksForwards(square, blockers, NORTH_WEST)
			   | _generateRayAttacksForwards(square, blockers, NORTH_EAST)
			   | _generateRayAttacksBackwards(square, blockers, SOUTH_EAST)
			   | _generateRayAttacksBackwards(square, blockers, SOUTH_WEST);
	}

	static constexpr u64 generateRookAttacks(const u8 square, const u64 blockers) noexcept
	{
		return _generateRayAttacksForwards(square, blockers, NORTH)
			   | _generateRayAttacksForwards(square, blockers, EAST)
			   | _generateRayAttacksBackwards(square, blockers, SOUTH)
			   | _generateRayAttacksBackwards(square, blockers, WEST);
	}

	static const auto LineBetweenSquares = []
	{
		std::array<std::array<u64, SQUARE_NB>, SQUARE_NB> array{};

		for (u8 sq1{}; sq1 < SQUARE_NB; ++sq1)
			for (u8 sq2{}; sq2 < SQUARE_NB; ++sq2)
			{
				const auto bb1 = Squares[sq1];
				const auto bb2 = Squares[sq2];

				if (generateRookAttacks(sq1, {}) & bb2)
					array[sq1][sq2] = generateRookAttacks(sq1, bb2) & generateRookAttacks(sq2, bb1);
				else if (generateBishopAttacks(sq1, {}) & bb2)
					array[sq1][sq2] = generateBishopAttacks(sq1, bb2) & generateBishopAttacks(sq2, bb1);
			}

		return array;
	}();

	constexpr u8 getDistanceBetween(const Square sq1, const Square sq2)
	{
		return DistanceBetweenSquares[u8(sq1)][u8(sq2)];
	}

// endregion Rays
}

class Bitboard
{
public:
	Bitboard() = default;

	explicit constexpr Bitboard(const u64 value) noexcept
		: _value(value) {}

	/// Const functions

	[[nodiscard]] constexpr auto value() const noexcept { return _value; }

	[[nodiscard]] constexpr bool empty() const noexcept { return _value == 0ull; }

	[[nodiscard]] constexpr bool notEmpty() const noexcept { return static_cast<bool>(_value); }

	[[nodiscard]] constexpr Square bitScanForward() const noexcept
	{
		return toSquare(Bits::bitScanForward(_value));
	}

	[[nodiscard]] constexpr i32 popcount() const noexcept
	{
		return std::popcount(_value);
	}

	template <Dir D, Dir... Dirs>
	[[nodiscard]] constexpr Bitboard shift() const noexcept
	{
		u64 result = Bits::_shiftT<D>(_value);
		((result = Bits::_shiftT<Dirs>(result)), ...);
		return Bitboard{ result };
	}

	[[nodiscard]] Bitboard flipVertical() const noexcept
	{
		return Bitboard{ Bits::flipVertical(_value) };
	}

	[[nodiscard]] constexpr bool several() const noexcept
	{
		return static_cast<bool>(_value & (_value - 1));
	}

	/// Non-Const functions

	constexpr Bitboard &addSquare(const Square square) noexcept
	{
		return (*this) |= Bitboard::fromSquare(square);
	}

	constexpr Bitboard &removeSquare(const Square square) noexcept
	{
		return (*this) &= ~Bitboard::fromSquare(square);
	}

	constexpr Square popLsb() noexcept
	{
		const Square lsbIndex = bitScanForward();
		_value &= _value - 1u;
		return lsbIndex;
	}

	/// Operators

	constexpr bool operator==(const Bitboard &rhs) const noexcept
	{
		return _value == rhs._value;
	}

	constexpr bool operator!=(const Bitboard &rhs) const noexcept
	{
		return _value != rhs._value;
	}


	constexpr Bitboard &operator|=(const Bitboard &rhs) noexcept
	{
		_value |= rhs._value;
		return *this;
	}

	constexpr Bitboard &operator&=(const Bitboard &rhs) noexcept
	{
		_value &= rhs._value;
		return *this;
	}

	constexpr Bitboard &operator^=(const Bitboard &rhs) noexcept
	{
		_value ^= rhs._value;
		return *this;
	}


	constexpr Bitboard operator|(const Bitboard &rhs) const noexcept
	{
		return Bitboard{ _value | rhs._value };
	}

	constexpr Bitboard operator&(const Bitboard &rhs) const noexcept
	{
		return Bitboard{ _value & rhs._value };
	}

	constexpr Bitboard operator^(const Bitboard &rhs) const noexcept
	{
		return Bitboard{ _value ^ rhs._value };
	}

	constexpr Bitboard operator~() const noexcept
	{
		return Bitboard{ ~_value };
	}

	constexpr Bitboard operator<<(const u64 rhs) const noexcept
	{
		return Bitboard{ _value << rhs };
	}

	constexpr Bitboard operator>>(const u64 rhs) const noexcept
	{
		return Bitboard{ _value >> rhs };
	}

	/// Static

public:
	static constexpr Bitboard fromSquare(const Square square) noexcept
	{
		return Bitboard{ Bits::Squares[u8(square)] };
	}

	/**
	 * Returns a bitboard containing the given ray in the given direction.
	 *
	 * @param direction Direction of ray to return
	 * @param square Square to get ray starting from (in little endian rank file mapping form)
	 */
	static constexpr Bitboard fromRay(const Dir direction, const Square square) noexcept
	{
		return Bitboard{ Bits::Rays[direction][u8(square)] };
	}

	static Bitboard fromLineBetween(const Square sq1, const Square sq2) noexcept
	{
		return Bitboard{ Bits::LineBetweenSquares[u8(sq1)][u8(sq2)] };
	}

	static Bitboard fromLine(const Square sq1, const Square sq2) noexcept
	{
		return fromLineBetween(sq1, sq2) | fromSquare(sq1) | fromSquare(sq2);
	}

	static constexpr Bitboard fromRank(const Square square) noexcept
	{
		return Bitboard{ Bits::Ranks[rankOf(square)] };
	}

	static constexpr Bitboard fromFile(const Square square) noexcept
	{
		return Bitboard{ Bits::Files[fileOf(square)] };
	}

	static constexpr Bitboard fromAdjacentFiles(const Square square) noexcept
	{
		const auto file = fromFile(square);
		return file.shift<WEST>() | file.shift<EAST>();
	}

	static bool areAligned(const Square sq1, const Square sq2, const Square sq3) noexcept
	{
		auto bitboard = fromLine(sq1, sq2);
		return (bitboard & fromSquare(sq3)).notEmpty();
	}

private:
	u64 _value{};
};

constexpr Bitboard RANK_1{ 0xFF };
constexpr Bitboard RANK_2{ RANK_1 << 8 };
constexpr Bitboard RANK_3{ RANK_1 << (8 * 2) };
constexpr Bitboard RANK_4{ RANK_1 << (8 * 3) };
constexpr Bitboard RANK_5{ RANK_1 << (8 * 4) };
constexpr Bitboard RANK_6{ RANK_1 << (8 * 5) };
constexpr Bitboard RANK_7{ RANK_1 << (8 * 6) };
constexpr Bitboard RANK_8{ RANK_1 << (8 * 7) };

constexpr Bitboard FILE_A{ 0x101010101010101ull };
constexpr Bitboard FILE_B{ FILE_A << 1 };
constexpr Bitboard FILE_C{ FILE_A << 2 };
constexpr Bitboard FILE_D{ FILE_A << 3 };
constexpr Bitboard FILE_E{ FILE_A << 4 };
constexpr Bitboard FILE_F{ FILE_A << 5 };
constexpr Bitboard FILE_G{ FILE_A << 6 };
constexpr Bitboard FILE_H{ FILE_A << 7 };

constexpr Bitboard ALL_SQUARES{ UINT64_MAX };
constexpr Bitboard DARK_SQUARES{ 0xAA55AA55AA55AA55ULL };

constexpr Bitboard KING_SIDE{ FILE_E | FILE_F | FILE_G | FILE_H };
constexpr Bitboard QUEEN_SIDE{ FILE_A | FILE_B | FILE_C | FILE_D };
constexpr Bitboard CENTER_FILES{ FILE_C | FILE_D | FILE_E | FILE_F };
constexpr Bitboard CENTER_SQUARES{ (FILE_D | FILE_E) & (RANK_4 | RANK_5) };