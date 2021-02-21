#pragma once

#include <algorithm>
#include <thread>

#include "Defs.h"

class Settings final
{
public:
	Settings() : Settings(8, 1, 64, true)
	{
	}

	Settings(const int depth,
			 const usize threadCount,
			 const usize tableSizeMb,
			 const bool performQuiescenceSearch,
			 const u64 searchTime = {}) noexcept
		: _depth(std::clamp<i32>(depth, 2, MAX_DEPTH)),
		  _threadCount(std::clamp<usize>(threadCount, 1u, std::thread::hardware_concurrency())),
		  _cacheTableSizeMb(tableSizeMb),
		  _searchTime(searchTime),
		  _quiescenceSearch(performQuiescenceSearch)
	{
	}

	constexpr auto depth() const noexcept
	{
		return _depth;
	}

	constexpr auto threadCount() const noexcept
	{
		return _threadCount;
	}

	constexpr auto tableSizeMb() const noexcept
	{
		return _cacheTableSizeMb;
	}

	constexpr bool isTimeSet() const noexcept
	{
		return _searchTime != 0ull;
	}

	constexpr auto searchTime() const noexcept
	{
		return _searchTime;
	}

	constexpr bool doQuietSearch() const noexcept
	{
		return _quiescenceSearch;
	}

private:
	i32 _depth;
	usize _threadCount;
	usize _cacheTableSizeMb;
	usize _searchTime;
	bool _quiescenceSearch;
};
