#include "Stats.h"

#include <sstream>

std::chrono::time_point<std::chrono::steady_clock> Stats::_startTime;
std::atomic_size_t Stats::_boardsEvaluated;
std::atomic_size_t Stats::_nodesSearched;
std::atomic_size_t Stats::_nodesGenerated;
std::atomic_size_t Stats::_betaCuts;
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
	_nodesGenerated = 0;
	_betaCuts = 0;
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

void Stats::incNodesGenerated(const std::size_t amount) noexcept
{
	if (_statsEnabled)
		_nodesGenerated += amount;
}

void Stats::incBetaCuts() noexcept
{
	if (_statsEnabled)
		++_betaCuts;
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

double Stats::getElapsedMs() noexcept
{
	return _elapsedTime;
}

std::string Stats::formatStats(const char separator) noexcept(false)
{
	std::stringstream stream;

	const double timeMs = getElapsedMs();

	stream << "Elapsed Time: " << timeMs << "ms" << separator;

	if (_statsEnabled)
	{
		const auto boardsEvaluated = static_cast<size_t>(_boardsEvaluated);
		const auto nodesSearched = static_cast<size_t>(_nodesSearched);
		const auto nodesGenerated = static_cast<size_t>(_nodesGenerated);
		const auto betaCuts = static_cast<size_t>(_betaCuts);
		const auto nullCuts = static_cast<size_t>(_nullCuts);
		const auto futilityCuts = static_cast<size_t>(_futilityCuts);
		const auto lmrCount = static_cast<size_t>(_lmrCount);
		const size_t nps = timeMs ? (nodesSearched / (timeMs / 1000.0)) : 0;

		stream << "Boards Evaluated: " << boardsEvaluated << separator
			   << "Nodes Searched: " << nodesSearched << separator
			   << "Nodes Generated: " << nodesGenerated << separator
			   << "Nps: " << nps << separator
			   << "Beta/Null: " << betaCuts << '/' << nullCuts << separator
			   << "Futility/LMR: " << futilityCuts << '/' << lmrCount << separator;
	}

	return stream.str();
}
