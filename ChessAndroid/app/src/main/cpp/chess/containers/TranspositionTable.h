#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>

struct Cache
{
    std::uint64_t key = 0;
};

struct EvaluationCache
{
    std::uint64_t key = 0;
    int score = 0;
};

struct SearchCache
{
    std::uint64_t key = 0;
    int score = 0;
    short depth = 0;
};

template<class T>
class TranspositionTable final
{
    using Key = std::uint64_t;

    const std::size_t m_Size;
    T *m_Values = new T[m_Size];
    mutable std::shared_mutex m_Mutex;

public:
    explicit TranspositionTable(const std::size_t size) noexcept
        : m_Size(size)
    {
        std::memset(m_Values, 0, sizeof(T) * m_Size);
    }

    T get(const Key key) const noexcept
    {
        std::shared_lock lock(m_Mutex);
        return m_Values[key % m_Size];
    }

    void insert(const T &value) noexcept
    {
        std::unique_lock lock(m_Mutex);
        m_Values[value.key % m_Size] = value;
    }
};
