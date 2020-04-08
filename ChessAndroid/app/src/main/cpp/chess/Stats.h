#pragma once

#include <atomic>
#include <chrono>
#include <string>

class Stats final
{
	inline static bool _statsEnabled = false;
	inline static double _elapsedTime = 0;
	static std::chrono::time_point<std::chrono::steady_clock> _startTime;

	static std::atomic_size_t _boardsEvaluated;
	static std::atomic_size_t _nodesSearched;
	static std::atomic_size_t _nodesGenerated;

public:
	Stats() = delete;
	Stats(const Stats&) = delete;
	Stats(Stats&&) = delete;

	static bool isEnabled() noexcept { return _statsEnabled; }
	static void setEnabled(bool enabled) noexcept;
	static void resetStats() noexcept;

	static size_t getBoardsEvaluated() noexcept;
	static size_t getNodesSearched() noexcept;
	static size_t getNodesGenerated() noexcept;

	static void incrementBoardsEvaluated() noexcept;
	static void incrementNodesSearched(std::size_t amount = 1u) noexcept;
	static void incrementNodesGenerated(std::size_t amount = 1u) noexcept;

	static void startTimer() noexcept;
	static void stopTimer() noexcept;
	static double getElapsedTime() noexcept;

	static std::string formatStats(char separator) noexcept(false);
};
