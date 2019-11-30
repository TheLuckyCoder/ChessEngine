#pragma once

enum Color : bool
{
	BLACK = false,
	WHITE = true
};

constexpr Color toColor(const bool isWhite)
{
	return isWhite ? WHITE : BLACK;
}

constexpr Color oppositeColor(const Color color)
{
	return color == BLACK ? WHITE : BLACK;
}

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
	CASTLE_WHITE_KING = 0b0001,
	CASTLE_WHITE_QUEEN = 0b0010,
	CASTLE_WHITE = CASTLE_WHITE_KING | CASTLE_WHITE_QUEEN,
	CASTLED_WHITE = 0b010000,

	CASTLE_BLACK_KING = 0b0100,
	CASTLE_BLACK_QUEEN = 0b1000,
	CASTLE_BLACK = CASTLE_BLACK_KING | CASTLE_BLACK_QUEEN,
	CASTLED_BLACK = 0b100000
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
