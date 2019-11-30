#pragma once

#include "../containers/TranspositionTable.h"

class Board;
class RootMove;
class Settings;

class Search final
{
	static bool s_QuiescenceSearchEnabled;
	static TranspositionTable s_SearchCache;
	static short s_BestMoveFound;

public:
	Search() = delete;
	Search(const Search&) = delete;
	Search(Search&&) = delete;

	static RootMove findBestMove(const Board &board, const Settings &settings);
	static short getBestMoveFound();

private:
	static RootMove negaMaxRoot(const std::vector<RootMove> &validMoves, unsigned int jobCount, short ply);
	static short negaMax(const Board &board, short ply, short alpha, short beta, short depth, bool moveCountPruning);
	static short quiescence(const Board &board, short alpha, short beta);
	static short negaScout(const Board &board, short ply, short alpha, short beta, bool isWhite, short depth);

	inline static short sideToMove(const Board &board);
};
