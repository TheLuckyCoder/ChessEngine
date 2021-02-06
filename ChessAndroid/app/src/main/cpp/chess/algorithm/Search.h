#pragma once

#include "../Settings.h"
#include "../Move.h"
#include "../TranspositionTable.h"

class Board;

class Search final
{
private:
	class SharedState
	{
	public:
		bool stopped{};
		std::atomic_uint64_t nodes{};

		mutable std::mutex mutex{};
		// Stats for the last time the depth was updated
		std::atomic_int depth{};
		int bestScore{};
		usize time{};

		// This should only be read and written by the main thread
		int lastReportedDepth{};
		Move lastReportedBestMove{};
	};

	static Settings _searchSettings;
	static TranspositionTable _transpositionTable;
	static SharedState _sharedState;

public:
	Search() = delete;
	Search(const Search &) = delete;
	Search(Search &&) = delete;

	Search &operator=(const Search &) = delete;
	Search &operator=(Search &&) = delete;

	static void clearAll();
	static void stopSearch();
	static bool setTableSize(usize sizeMb);

	static Move findBestMove(Board board, const Settings &settings);

	static auto &getTranspTable() noexcept { return _transpositionTable; }

private:
	static void printUci(Board &board);
	static void iterativeDeepening(Board board, int targetDepth);
	static int aspirationWindow(Board &board, int depth, int bestScore);
	static int search(Board &board, int alpha, int beta, int depth, bool isPvNode,
					  bool doNull, bool doLmr);
	static int searchCaptures(Board &board, int alpha, int beta, int depth);

	inline static void storeTtEntry(const Move &bestMove, u64 key, int alpha, int originalAlpha,
									int beta, int depth, bool qSearch);
	static bool checkTimeAndStop();
};
