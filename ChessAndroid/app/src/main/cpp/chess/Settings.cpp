#include "Settings.h"

#include <thread>

Settings::Settings(const short baseSearchDepth,
				   const unsigned int threadCount,
				   const unsigned int cacheTableSizeMb,
				   const bool performQuiescenceSearch) noexcept
		: cacheTableSizeMb(cacheTableSizeMb), quiescenceSearch(performQuiescenceSearch)
{
	const auto maxThreads = std::thread::hardware_concurrency();

	if (threadCount > maxThreads)
		this->threadCount = maxThreads;
	else if (threadCount == 0)
		this->threadCount = 1u;
	else
		this->threadCount = threadCount;

	this->baseSearchDepth = baseSearchDepth < 1 ? true : baseSearchDepth;
}

short Settings::getBaseSearchDepth() const noexcept
{
	return baseSearchDepth;
}

unsigned int Settings::getThreadCount() const noexcept
{
	return threadCount;
}

unsigned int Settings::getCacheTableSizeMb() const noexcept
{
	return cacheTableSizeMb;
}

bool Settings::performQuiescenceSearch() const noexcept
{
	return quiescenceSearch;
}
