#include "Stats.h"

#include <sstream>

std::chrono::time_point<std::chrono::steady_clock> Stats::_startTime;
std::atomic_size_t Stats::boardsEvaluated;
std::atomic_size_t Stats::nodesSearched;

void Stats::setEnabled(const bool enabled) noexcept
{
	_statsEnabled = enabled;
}

void Stats::resetStats() noexcept
{
	boardsEvaluated = 0;
	nodesSearched = 0;
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

std::string Stats::formatStats(const char separator) noexcept(false)
{
	std::stringstream stream;

	stream << "Boards Evaluated: " << static_cast<size_t>(boardsEvaluated) << separator
		<< "Nodes Searched: " << static_cast<size_t>(nodesSearched) << separator
		<< "Time Needed: " << _elapsedTime << " millis" << separator;

	return stream.str();
}
