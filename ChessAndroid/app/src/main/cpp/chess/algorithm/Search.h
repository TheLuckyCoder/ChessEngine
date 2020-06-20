#pragma once

#include <array>

#include "../Settings.h"
#include "../Move.h"
#include "../containers/TranspositionTable.h"

class Board;

class Search final
{
	struct State
	{
		bool stopped{};
		bool doQuietSearch = true;
		bool useTime{};
		size_t time{};
		U64 nodes{};
	};

	static TranspositionTable _transpTable;
	static std::array<std::array<unsigned int, MAX_DEPTH>, 2> _searchKillers;
	static std::array<std::array<byte, SQUARE_NB>, SQUARE_NB> _searchHistory;
	static std::array<int, MAX_DEPTH> _evalHistory;
	static State _state;

public:
	Search() = delete;
	Search(const Search &) = delete;
	Search(Search &&) = delete;

	Search &operator=(const Search &) = delete;
	Search &operator=(Search &&) = delete;

	static void clearAll();
	static void stopSearch();
	static bool setTableSize(size_t sizeMb);

	static Move findBestMove(Board board, const Settings &settings);

	static auto &getTranspTable() noexcept { return _transpTable; }

	static auto &getSearchKillers() noexcept { return _searchKillers; }

	static auto &getSearchHistory() noexcept { return _searchHistory; }

private:
	static Move iterativeDeepening(Board &board, int depth);
	static int aspirationWindow(Board &board, int depth, int bestScore);
	static int
	search(Board &board, int alpha, int beta, int depth, bool isPvNode, bool doNull, bool doLmr);
	static int searchCaptures(Board &board, int alpha, int beta, int depth);

	inline static void
	storeTtEntry(const Move &bestMove, U64 key, int alpha, int originalAlpha, int beta, int depth,
				 bool qSearch);
	static bool checkTimeAndStop();
};
