#pragma once

#include "../data/Pos.h"

class Board;
class Settings;

class NegaMax final
{
public:
	NegaMax() = delete;

	static PosPair negaMax(const Board &board, bool isWhite, const Settings &settings);

private:
	static int negaMax(const Board &board, short depth, int alpha, int beta, bool isWhite, bool extended);
	static int quiescence(const Board &board, short depth, int alpha, int beta, bool isWhite);
};
