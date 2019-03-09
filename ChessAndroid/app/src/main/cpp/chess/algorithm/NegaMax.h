#pragma once

#include <mutex>

#include "../data/Pos.h"
#include "../data/Board.h"
#include "../containers/TranspositionTable.h"

class Board;
class Settings;

class NegaMax final
{
	inline static TranspositionTable<SearchCache> searchCache{ 10000000 };

public:
	NegaMax() = delete;

	static PosPair getBestMove(const Board &board, bool isWhite, const Settings &settings, bool firstMove);

private:
	static Move negaMaxRoot(StackVector<Move, 150> validMoves, unsigned int jobCount, short depth, bool isWhite);
	static int negaMax(const Board &board, short depth, int alpha, int beta, bool isWhite, bool extended);
	static int quiescence(const Board &board, short depth, int alpha, int beta, bool isWhite);
};
