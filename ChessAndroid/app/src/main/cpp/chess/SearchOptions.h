#pragma once

#include <algorithm>
#include <thread>

#include "Defs.h"

class SearchOptions final
{
public:

	SearchOptions() : SearchOptions(6, std::max<usize>(1u, std::thread::hardware_concurrency() / 2), 64, true)
	{
	}

	SearchOptions(const int depth,
				  const usize threadCount,
				  const usize tableSizeMb,
				  const bool quietSearch,
				  const u64 searchTime = {}) noexcept
		: _depth(std::clamp<i32>(depth, 2, MAX_DEPTH)),
		  _threadCount(std::clamp<usize>(threadCount, 1u, std::thread::hardware_concurrency())),
		  _cacheTableSizeMb(tableSizeMb),
		  _searchTime(searchTime),
		  _quiescenceSearch(quietSearch)
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

	constexpr bool quietSearch() const noexcept
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
