#pragma once

#include "../data/Pos.h"
#include "../data/Board.h"
#include "../containers/TranspositionTable.h"

class Board;
class Settings;

class NegaMax final
{
	static bool quiescenceSearchEnabled;
	static TranspositionTable<SearchCache> searchCache;

public:
	NegaMax() = delete;

	static PosPair getBestMove(const Board &board, bool isWhite, const Settings &settings);

private:
	static RootMove negaMaxRoot(StackVector<RootMove, 150> validMoves, unsigned int jobCount, short depth, bool isWhite);
	static int negaMax(const Board &board, short depth, int alpha, int beta, bool isWhite, bool extended);
	static int quiescence(const Board &board, int alpha, int beta, bool isWhite);
	static int negaScout(const Board &board, short depth, int alpha, int beta, bool isWhite);

	inline static int sideToMove(const Board &board, bool isWhite);
};
