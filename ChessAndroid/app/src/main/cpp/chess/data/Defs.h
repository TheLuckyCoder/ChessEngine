#pragma once

#include <cstdint>
#include <cstddef>

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

enum Dir : std::int16_t
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

enum class State : byte
{
	NONE,
	WINNER_WHITE,
	WINNER_BLACK,
	DRAW,
	WHITE_IN_CHECK,
	BLACK_IN_CHECK,
	INVALID = 10
};

enum CastlingRights : byte
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

enum Phase : byte
{
	END_GAME_PHASE,
	MIDDLE_GAME_PHASE = 128
};

enum Value : short
{
	VALUE_MAX = 32001,
	VALUE_MIN = -VALUE_MAX,

	VALUE_MATE_WHITE = VALUE_MAX - 1,
	VALUE_MATE_BLACK = -VALUE_MATE_WHITE,

	MAX_MOVES = 256,
	MAX_DEPTH = 64
};

constexpr bool isMateValue(const Value value) noexcept
{
	const int absValue = value < 0 ? -value : value;
	return (absValue + MAX_DEPTH + 1) > VALUE_MAX;
}

enum PieceType : byte
{
	NO_PIECE_TYPE = 0,
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

enum Square : byte
{
	SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
	SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
	SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
	SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
	SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
	SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
	SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
	SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,

	SQ_NONE = 64,
	SQUARE_NB = 64
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

constexpr byte row(const byte pos) noexcept { return static_cast<byte>(pos >> 3u); }

constexpr byte col(const byte pos) noexcept { return static_cast<byte>(pos & 7u); }

constexpr byte toSquare(const byte x, const byte y) noexcept
{
	return static_cast<byte>((y << 3u) + x);
}
