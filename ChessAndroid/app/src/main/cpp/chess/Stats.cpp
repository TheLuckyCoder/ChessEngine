#include "Stats.h"

#include <sstream>

std::chrono::time_point<std::chrono::steady_clock> Stats::_startTime;
std::atomic_size_t Stats::_boardsEvaluated;
std::atomic_size_t Stats::_nodesSearched;
std::atomic_size_t Stats::_nodesGenerated;

void Stats::setEnabled(const bool enabled) noexcept
{
	_statsEnabled = enabled;
}

void Stats::resetStats() noexcept
{
	_boardsEvaluated = 0;
	_nodesSearched = 0;
	_nodesGenerated = 0;
}

size_t Stats::getBoardsEvaluated() noexcept
{
	return _boardsEvaluated;
}

size_t Stats::getNodesSearched() noexcept
{
	return _nodesSearched;
}

size_t Stats::getNodesGenerated() noexcept
{
	return _nodesGenerated;
}

void Stats::incrementBoardsEvaluated() noexcept
{
	if (_statsEnabled)
		++_boardsEvaluated;
}

void Stats::incrementNodesSearched(const std::size_t amount) noexcept
{
	if (_statsEnabled)
		_nodesSearched += amount;
}

void Stats::incrementNodesGenerated(const std::size_t amount) noexcept
{
	if (_statsEnabled)
		_nodesGenerated += amount;
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

	stream << "Boards Evaluated: " << static_cast<size_t>(_boardsEvaluated) << separator
		   << "Nodes Searched: " << static_cast<size_t>(_nodesSearched) << separator
		   << "Nodes Generated: " << static_cast<size_t>(_nodesGenerated) << separator;

	return stream.str();
}
