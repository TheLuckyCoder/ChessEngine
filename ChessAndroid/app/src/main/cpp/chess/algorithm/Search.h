#pragma once

#include <array>

#include "../Settings.h"
#include "../data/Move.h"
#include "../containers/TranspositionTable.h"
#include "../containers/ThreadPool.h"

class Board;

class Search final
{
	static TranspositionTable _transpTable;
	static std::array<std::array<Move, MAX_DEPTH>, 2> _searchKillers;
	static std::array<std::array<int, 64>, 64> _searchHistory;
	static ThreadPool _threadPool;
	static bool _quiescenceSearchEnabled;

public:
	Search() = delete;
	Search(const Search&) = delete;
	Search(Search&&) = delete;

	Search &operator=(const Search&) = delete;
	Search &operator=(Search&&) = delete;

	static Move findBestMove(Board board, const Settings &settings);
	
	static auto &getTranspTable() noexcept { return _transpTable; }
	static auto &getSearchKillers() noexcept { return _searchKillers; }
	static auto &getSearchHistory() noexcept { return _searchHistory; }

private:
	static Move iterativeDeepening(Board &board, const int depth);
	static int search(Board &board, int alpha, int beta, int depth, bool doNull, bool doMoveCountPruning);
	static int searchCaptures(Board &board, int alpha, int beta);

	inline static int evaluate(const Board &board);
};
