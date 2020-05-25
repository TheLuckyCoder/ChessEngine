#pragma once

#include <algorithm>
#include <thread>

#include "Defs.h"

class Settings final
{
public:
	Settings(const std::size_t depth,
	         const std::size_t threadCount,
	         const std::size_t tableSizeMb,
	         const bool performQuiescenceSearch,
	         const U64 searchTime = 0ull) noexcept
		: _baseSearchDepth(std::clamp<std::size_t>(depth, 2u, MAX_DEPTH)),
		  _threadCount(std::clamp<std::size_t>(threadCount, 1u, std::thread::hardware_concurrency())),
		  _cacheTableSizeMb(tableSizeMb),
		  _searchTime(searchTime),
		  _quiescenceSearch(performQuiescenceSearch)
	{
	}

	std::size_t getSearchDepth() const noexcept
	{
		return _baseSearchDepth;
	}

	std::size_t getThreadCount() const noexcept
	{
		return _threadCount;
	}

	std::size_t getTableSizeMb() const noexcept
	{
		return _cacheTableSizeMb;
	}

	std::size_t getSearchTime() const noexcept
	{
		return _searchTime;
	}

	bool doQuiescenceSearch() const noexcept
	{
		return _quiescenceSearch;
	}

private:
	std::size_t _baseSearchDepth;
	std::size_t _threadCount;
	std::size_t _cacheTableSizeMb;
	std::size_t _searchTime;
	bool _quiescenceSearch;
};
