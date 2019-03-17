#pragma once

class Settings final
{
    short baseSearchDepth;
    unsigned int threadCount;
    unsigned int cacheTableSizeMb;

public:
    Settings(short baseSearchDepth, unsigned int threadCount, unsigned int cacheTableSizeMb) noexcept;

    short getBaseSearchDepth() const noexcept;
    unsigned int getThreadCount() const noexcept;
    unsigned int getCacheTableSizeMb() const noexcept;
};
