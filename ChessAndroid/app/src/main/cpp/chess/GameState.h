#pragma once

enum class GameState : unsigned char
{
    NONE,
    WINNER_WHITE,
    WINNER_BLACK,
    DRAW,
	WHITE_IN_CHESS,
	BLACK_IN_CHESS
};
