#pragma once

#include <cstdint>

using byte = std::uint8_t;
using U64 = std::uint64_t;

enum Color : bool
{
	BLACK = false,
	WHITE = true
};

constexpr Color operator~(const Color c) noexcept
{
	return Color(c ^ WHITE); // Toggle color
}

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

enum class State : unsigned char
{
	NONE,
	WINNER_WHITE,
	WINNER_BLACK,
	DRAW,
	WHITE_IN_CHECK,
	BLACK_IN_CHECK,
	INVALID = 10
};

enum CastlingRights : unsigned char
{
	CASTLE_NONE = 0,

	CASTLE_BLACK_KING = 0b0001,
	CASTLE_BLACK_QUEEN = 0b0010,
	CASTLE_BLACK_BOTH = CASTLE_BLACK_KING | CASTLE_BLACK_QUEEN,
	CASTLED_BLACK = 0b0100,

	CASTLE_WHITE_KING = 0b0'1000,
	CASTLE_WHITE_QUEEN = 0b1'0000,
	CASTLE_WHITE_BOTH = CASTLE_WHITE_KING | CASTLE_WHITE_QUEEN,
	CASTLED_WHITE = 0b10'0000
};

enum class Phase : short
{
	ENDING,
	MIDDLE = 128
};

enum Value : short
{
	VALUE_MAX = 32767,
	VALUE_MIN = -VALUE_MAX,

	VALUE_WINNER_WHITE = VALUE_MAX - 1,
	VALUE_WINNER_BLACK = -VALUE_WINNER_WHITE
};

enum class Flag : unsigned char
{
	EXACT,
	ALPHA,
	BETA
};

enum PieceType : unsigned char
{
	NO_PIECE_TYPE = 0,
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6,

	PIECE_TYPE_NB = 8
};

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

constexpr byte SQUARE_NB = 64u;

constexpr byte row(const byte pos) noexcept { return static_cast<byte>(pos >> 3u); }

constexpr byte col(const byte pos) noexcept { return static_cast<byte>(pos & 7u); }

constexpr byte toSquare(const byte x, const byte y) noexcept
{
	return static_cast<byte>((y << 3u) + x);
}
