#include "Settings.h"

#include <thread>

Settings::Settings(const short baseSearchDepth, const unsigned int threadCount) noexcept
    : baseSearchDepth(baseSearchDepth)
{
    const auto maxThreads = std::thread::hardware_concurrency();

	if (threadCount > maxThreads)
		this->threadCount = maxThreads;
	else if (threadCount == 0)
		this->threadCount = 1u;
	else
		this->threadCount = threadCount;
}

short Settings::getBaseSearchDepth() const noexcept
{
    return baseSearchDepth;
}

unsigned int Settings::getThreadCount() const noexcept
{
    return threadCount;
}
