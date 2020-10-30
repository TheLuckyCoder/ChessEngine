#pragma once

#include <algorithm>
#include <thread>

#include "Defs.h"

class Settings final
{
public:
	Settings(const int depth,
	         const std::size_t threadCount,
	         const std::size_t tableSizeMb,
	         const bool performQuiescenceSearch,
	         const u64 searchTime = 0ull) noexcept
		: _depth(std::clamp<int>(depth, 2, MAX_DEPTH)),
		  _threadCount(std::clamp<std::size_t>(threadCount, 1u, std::thread::hardware_concurrency())),
		  _cacheTableSizeMb(tableSizeMb),
		  _searchTime(searchTime),
		  _quiescenceSearch(performQuiescenceSearch)
	{
	}

	int depth() const noexcept
	{
		return _depth;
	}

	std::size_t threadCount() const noexcept
	{
		return _threadCount;
	}

	std::size_t tableSizeMb() const noexcept
	{
		return _cacheTableSizeMb;
	}

	bool isTimeSet() const noexcept
	{
		return _searchTime != 0ull;
	}

	std::size_t searchTime() const noexcept
	{
		return _searchTime;
	}

	bool doQuietSearch() const noexcept
	{
		return _quiescenceSearch;
	}

private:
	int _depth;
	std::size_t _threadCount;
	std::size_t _cacheTableSizeMb;
	std::size_t _searchTime;
	bool _quiescenceSearch;
};
