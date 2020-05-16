#pragma once

#include <algorithm>
#include <thread>

#include "data/Defs.h"

class Settings final
{
public:
	Settings(const std::size_t maxPlys,
	         const std::size_t threadCount,
	         const std::size_t cacheTableSizeMb,
	         const bool performQuiescenceSearch,
	         const U64 searchTime = 0ull) noexcept
		: _baseSearchDepth(std::clamp<std::size_t>(maxPlys, 1u, MAX_DEPTH)),
		  _threadCount(std::clamp<std::size_t>(threadCount, 1u, std::thread::hardware_concurrency())),
		  _cacheTableSizeMb(std::max<std::size_t>(cacheTableSizeMb, 1u)),
		  _quiescenceSearch(performQuiescenceSearch),
		  _searchTime(searchTime)
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

	std::size_t getCacheTableSizeMb() const noexcept
	{
		return _cacheTableSizeMb;
	}

	bool doQuiescenceSearch() const noexcept
	{
		return _quiescenceSearch;
	}

	U64 getSearchTime() const noexcept
	{
		return _searchTime;
	}

private:
	std::size_t _baseSearchDepth;
	std::size_t _threadCount;
	std::size_t _cacheTableSizeMb;
	bool _quiescenceSearch;
	U64 _searchTime;
};
