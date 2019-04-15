#pragma once

class Settings final
{
    short baseSearchDepth;
    unsigned int threadCount;
    unsigned int cacheTableSizeMb;
    bool quiescenceSearch;

public:
    Settings(short baseSearchDepth, unsigned int threadCount, unsigned int cacheTableSizeMb, bool performQuiescenceSearch) noexcept;

    short getBaseSearchDepth() const noexcept;
    unsigned int getThreadCount() const noexcept;
    unsigned int getCacheTableSizeMb() const noexcept;
    bool performQuiescenceSearch() const noexcept;
};
