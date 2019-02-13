#pragma once

enum class State : unsigned char
{
	NONE,
	WINNER_WHITE,
	WINNER_BLACK,
	DRAW,
	WHITE_IN_CHESS,
	BLACK_IN_CHESS
};

enum class Phase
{
	ENDING,
	MIDDLE = 128
};

enum Value : int
{
	VALUE_MAX = 2000000000,
	VALUE_MIN = -VALUE_MAX,
	VALUE_WINNER_WHITE = 32767,
	VALUE_WINNER_BLACK = -VALUE_WINNER_WHITE
};
