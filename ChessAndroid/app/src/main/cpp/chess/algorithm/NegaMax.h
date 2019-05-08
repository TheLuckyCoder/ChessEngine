#pragma once

#include "../data/Pos.h"
#include "../data/Board.h"
#include "../containers/TranspositionTable.h"

class Board;
class Settings;

class NegaMax final
{
	static bool quiescenceSearchEnabled;
	static TranspositionTable searchCache;

public:
	static short bestMoveFound;

	NegaMax() = delete;
	NegaMax(const NegaMax&) = delete;
	NegaMax(NegaMax&&) = delete;

	static RootMove getBestMove(const Board &board, bool isWhite, const Settings &settings);

private:
	static RootMove negaMaxRoot(StackVector<RootMove, 150> validMoves, unsigned int jobCount, short ply, bool isWhite);
	static short negaMax(const Board &board, short ply, short alpha, short beta, bool isWhite, short depth, bool moveCountPruning);
	static short quiescence(const Board &board, short alpha, short beta, bool isWhite);
	static short negaScout(const Board &board, short ply, short alpha, short beta, bool isWhite, short depth);

	inline static short sideToMove(const Board &board, bool isWhite);
};
