#pragma once

#include <atomic>
#include <chrono>
#include <string>

class Stats final
{
	static std::chrono::time_point<std::chrono::steady_clock> _startTime;
	static double _elapsedTime;
public:
	Stats() = delete;

	static std::atomic_size_t boardsEvaluated;
	static std::atomic_size_t nodesSearched;
	static std::atomic_size_t allocationsCount;
	static std::atomic_size_t allocatedMemory;

	static void resetStats() noexcept;
	static void startTimer() noexcept;
	static void stopTimer() noexcept;
	static double getElapsedTime() noexcept;
	static std::string formatStats(char separator) noexcept;
};
