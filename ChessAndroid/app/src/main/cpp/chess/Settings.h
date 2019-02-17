#pragma once

class Settings final
{
    short baseSearchDepth;
    unsigned int threadCount;

public:
    explicit Settings(short baseSearchDepth, unsigned int threadCount = 1u) noexcept;

    short getBaseSearchDepth() const noexcept;
    unsigned int getThreadCount() const noexcept;
};
