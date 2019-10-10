#pragma once

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

enum class CastlingRights : unsigned char
{
	NONE,
	CAN_CASTLE_KING,
	CAN_CASTLE_QUEEN,
	CAN_CASTLE_BOTH,
	CASTLED
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
