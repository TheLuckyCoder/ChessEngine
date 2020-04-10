#pragma once

#include <algorithm>
#include <thread>

#include "data/Defs.h"

class Settings final
{
public:
	Settings(const std::size_t baseSearchDepth,
	         const std::size_t threadCount,
	         const std::size_t cacheTableSizeMb,
	         const bool performQuiescenceSearch) noexcept
		: _baseSearchDepth(std::clamp<std::size_t>(baseSearchDepth, 1u, MAX_DEPTH)),
		  _threadCount(std::clamp<std::size_t>(threadCount, 1u, std::thread::hardware_concurrency())),
		  _cacheTableSizeMb(std::max<std::size_t>(cacheTableSizeMb, 1u)),
		  _quiescenceSearch(performQuiescenceSearch)
	{
	}

	std::size_t getBaseSearchDepth() const noexcept
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

	bool performQuiescenceSearch() const noexcept
	{
		return _quiescenceSearch;
	}

private:
	std::size_t _baseSearchDepth;
	std::size_t _threadCount;
	std::size_t _cacheTableSizeMb;
	bool _quiescenceSearch;
};
