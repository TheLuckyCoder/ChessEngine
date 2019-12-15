#include "Stats.h"

#include <sstream>

std::chrono::time_point<std::chrono::steady_clock> Stats::s_StartTime;
std::atomic_size_t Stats::s_BoardsEvaluated;
std::atomic_size_t Stats::s_NodesSearched;
std::atomic_size_t Stats::s_NodesGenerated;

void Stats::setEnabled(const bool enabled) noexcept
{
	s_StatsEnabled = enabled;
}

void Stats::resetStats() noexcept
{
	s_BoardsEvaluated = 0;
	s_NodesSearched = 0;
	s_NodesGenerated = 0;
}

size_t Stats::getBoardsEvaluated() noexcept
{
	return s_BoardsEvaluated;
}

size_t Stats::getNodesSearched() noexcept
{
	return s_NodesSearched;
}

size_t Stats::getNodesGenerated() noexcept
{
	return s_NodesGenerated;
}

void Stats::incrementBoardsEvaluated() noexcept
{
	if (s_StatsEnabled)
		++s_BoardsEvaluated;
}

void Stats::incrementNodesSearched(const std::size_t amount) noexcept
{
	if (s_StatsEnabled)
		s_NodesSearched += amount;
}

void Stats::incrementNodesGenerated(const std::size_t amount) noexcept
{
	if (s_StatsEnabled)
		s_NodesGenerated += amount;
}

void Stats::startTimer() noexcept
{
	s_StartTime = std::chrono::high_resolution_clock::now();
	s_ElapsedTime = 0;
}

void Stats::stopTimer() noexcept
{
	const auto currentTime = std::chrono::high_resolution_clock::now();
	s_ElapsedTime = std::chrono::duration<double, std::milli>(currentTime - s_StartTime).count();
}

double Stats::getElapsedTime() noexcept
{
	return s_ElapsedTime;
}

std::string Stats::formatStats(const char separator) noexcept(false)
{
	std::stringstream stream;

	stream << "Boards Evaluated: " << static_cast<size_t>(s_BoardsEvaluated) << separator
		<< "Nodes Searched: " << static_cast<size_t>(s_NodesSearched) << separator
		<< "Nodes Generated: " << static_cast<size_t>(s_NodesGenerated) << separator;

	return stream.str();
}
