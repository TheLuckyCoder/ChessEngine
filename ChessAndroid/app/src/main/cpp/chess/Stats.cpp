#include "Stats.h"

#include <sstream>

void *operator new(const std::size_t _Size) noexcept(false)
{
	++Stats::allocationsCount;
	Stats::allocatedMemory += _Size;
	return std::malloc(_Size);
}

void *operator new[](const std::size_t _Size) noexcept(false)
{
	++Stats::allocationsCount;
	Stats::allocatedMemory += _Size;
	return std::malloc(_Size);
}

std::chrono::time_point<std::chrono::steady_clock> Stats::_startTime;
double Stats::_elapsedTime;
std::atomic_size_t Stats::boardsEvaluated;
std::atomic_size_t Stats::nodesSearched;
std::atomic_size_t Stats::allocationsCount;
std::atomic_size_t Stats::allocatedMemory;

void Stats::resetStats() noexcept
{
	boardsEvaluated = 0;
	nodesSearched = 0;
	allocationsCount = 0;
	allocatedMemory = 0;
}

void Stats::startTimer() noexcept
{
	_startTime = std::chrono::high_resolution_clock::now();
	_elapsedTime = 0;
}

void Stats::stopTimer() noexcept
{
	const auto currentTime = std::chrono::high_resolution_clock::now();
	_elapsedTime = std::chrono::duration<double, std::milli>(currentTime - _startTime).count();
}

double Stats::getElapsedTime() noexcept
{
	return _elapsedTime;
}

std::string Stats::formatStats(const char separator) noexcept
{
	std::stringstream stream;

	stream << "Boards Evaluated: " << static_cast<size_t>(boardsEvaluated) << separator
		<< "Nodes Searched: " << static_cast<size_t>(nodesSearched) << separator
		<< "Time Needed: " << _elapsedTime << " millis" << separator
		<< "Allocations: " << static_cast<size_t>(allocationsCount) << separator
		<< "Allocated Memory: " << static_cast<size_t>(allocatedMemory) << separator;

	return stream.str();
}
