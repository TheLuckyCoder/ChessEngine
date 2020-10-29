#pragma once

#include <array>
#include <cstdint>

#include "../Settings.h"
#include "../Move.h"
#include "../containers/TranspositionTable.h"

#include "../Thread.h"

class Board;

class Search final
{
private:
	struct State
	{
		std::atomic_bool stopped{};
		std::atomic_uint64_t nodes{};
		bool doQuietSearch = true;
		bool useTime{};
		size_t time{};
		int maxDepth{};
	};

	struct DepthCounter
	{
		mutable std::mutex mutex{};
		// Stats for the last time the depth was updated
		std::atomic_int depth{};
		int bestScore{};
		size_t time{};

		// This should only be read and written by the main thread
		int lastReportedDepth{};
	};

	static TranspositionTable _transpTable;
	static DepthCounter _depthCounter;
	static State _state;

public:
	Search() = delete;
	Search(const Search &) = delete;
	Search(Search &&) = delete;

	Search &operator=(const Search &) = delete;
	Search &operator=(Search &&) = delete;

	static void clearAll();
	static void stopSearch();
	static bool setTableSize(std::size_t sizeMb);

	static Move findBestMove(Board board, const Settings &settings);

	static auto &getTranspTable() noexcept { return _transpTable; }

private:
	static void printUci(Board &board);
	static void iterativeDeepening(Board board, int targetDepth);
	static int aspirationWindow(Board &board, int depth, int bestScore);
	template <bool PvNode>
	static int search(Board &board, int alpha, int beta, int depth,
					  bool doNull, bool doLmr);
	static int searchCaptures(Board &board, int alpha, int beta, int depth);

	inline static void storeTtEntry(const Move &bestMove, U64 key, int alpha, int originalAlpha,
									int beta, int depth, bool qSearch);
	static bool checkTimeAndStop();
};
