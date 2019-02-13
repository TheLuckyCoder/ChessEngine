#pragma once

#include <atomic>
#include <chrono>
#include <string>

class Stats final
{
	static std::chrono::time_point<std::chrono::steady_clock> _startTime;
	static double _elapsedTime;
public:
	static std::atomic_size_t boardsEvaluated;
	static std::atomic_size_t nodesSearched;
	static std::atomic_size_t allocationsCount;
	static std::atomic_size_t allocatedMemory;

	static void resetStats();
	static void startTimer();
	static void stopTimer();
	static double getElapsedTime();
	static std::string formatStats(char separator);
};
