#include "Stats.h"

#include <sstream>

std::chrono::time_point<std::chrono::steady_clock> Stats::_startTime;
std::atomic_size_t Stats::_boardsEvaluated;
std::atomic_size_t Stats::_nodesSearched;
std::atomic_size_t Stats::_nullCuts;
std::atomic_size_t Stats::_futilityCuts;
std::atomic_size_t Stats::_lmrCount;

void Stats::setEnabled(const bool enabled) noexcept
{
	_statsEnabled = enabled;
}

void Stats::resetStats() noexcept
{
	_boardsEvaluated = 0;
	_nodesSearched = 0;
	_nullCuts = 0;
	_futilityCuts = 0;
	_lmrCount = 0;
}

void Stats::incBoardsEvaluated() noexcept
{
	if (_statsEnabled)
		++_boardsEvaluated;
}

void Stats::incNodesSearched(const std::size_t amount) noexcept
{
	if (_statsEnabled)
		_nodesSearched += amount;
}

void Stats::incNullCuts() noexcept
{
	if (_statsEnabled)
		++_nullCuts;
}

void Stats::incFutilityCuts() noexcept
{
	if (_statsEnabled)
		++_futilityCuts;
}

void Stats::incLmrCount() noexcept
{
	if (_statsEnabled)
		++_lmrCount;
}

size_t Stats::getNodesCount() noexcept
{
	return static_cast<size_t>(_nodesSearched);
}

void Stats::restartTimer() noexcept
{
	_startTime = std::chrono::high_resolution_clock::now();
}

double Stats::getElapsedMs() noexcept
{
	const auto currentTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double, std::milli>(currentTime - _startTime).count();;
}

std::string Stats::formatStats(const char separator) noexcept(false)
{
	std::stringstream stream;

	const size_t timeMs = getElapsedMs();

	stream << "Elapsed Time: " << timeMs << "ms" << separator;

	if (_statsEnabled)
	{
		const auto boardsEvaluated = static_cast<size_t>(_boardsEvaluated);
		const auto nodesSearched = static_cast<size_t>(_nodesSearched);
		const auto nullCuts = static_cast<size_t>(_nullCuts);
		const auto futilityCuts = static_cast<size_t>(_futilityCuts);
		const auto lmrCount = static_cast<size_t>(_lmrCount);
		const size_t nps = timeMs ? (nodesSearched / (timeMs / 1000.0)) : 0;

		stream << "Boards Evaluated: " << boardsEvaluated << separator
			   << "Nodes Searched: " << nodesSearched << separator
			   << "Nps: " << nps << separator
			   << "Null: " << nullCuts << separator
			   << "Futility/LMR: " << futilityCuts << '/' << lmrCount << separator;
	}

	return stream.str();
}
