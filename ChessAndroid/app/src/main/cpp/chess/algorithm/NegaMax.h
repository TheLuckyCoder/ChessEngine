#pragma once

#include "../data/Pos.h"
#include "../data/Board.h"

class Board;
class Settings;

class NegaMax final
{
public:
	NegaMax() = delete;

	static PosPair getBestMove(const Board &board, bool isWhite, const Settings &settings, bool firstMove);

private:
	static void processWork(StackVector<Move, 150> &validMoves, Move &bestMove, int &alpha, unsigned int jobCount, short depth, bool isWhite);
	static int negaMax(const Board &board, short depth, int alpha, int beta, bool isWhite, bool extended);
	static int quiescence(const Board &board, short depth, int alpha, int beta, bool isWhite);
};
