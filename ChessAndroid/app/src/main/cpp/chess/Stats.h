#pragma once

#include <atomic>
#include <chrono>
#include <string>

class Stats final
{
	inline static bool _statsEnabled = false;
	static std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;

	static std::atomic_size_t _boardsEvaluated;
	static std::atomic_size_t _nodesSearched;
	static std::atomic_size_t _nullCuts;
	static std::atomic_size_t _futilityCuts;
	static std::atomic_size_t _lmrCount;

public:
	Stats() = delete;
	Stats(const Stats &) = delete;
	Stats(Stats &&) = delete;

	static bool isEnabled() noexcept { return _statsEnabled; }

	static void setEnabled(bool enabled) noexcept;
	static void resetStats() noexcept;

	static void incBoardsEvaluated() noexcept;
	static void incNodesSearched(std::size_t amount = 1u) noexcept;
	static void incNullCuts() noexcept;
	static void incFutilityCuts() noexcept;
	static void incLmrCount() noexcept;

	static size_t getNodesCount() noexcept;

	static void restartTimer() noexcept;
	static double getElapsedMs() noexcept;

	static std::string formatStats(char separator) noexcept(false);
};
