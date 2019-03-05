#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>

using U64 = std::uint64_t;
using byte = unsigned char;

struct SearchCache
{
    U64 key = 0;
    int score = 0, bestMove = 0;
    byte depth = 0;
};

template<class T>
class TranspositionTable final
{
    const std::size_t m_Size;
    T *m_Values = new T[m_Size];
    mutable std::shared_mutex m_Mutex;

public:
    explicit TranspositionTable(const std::size_t size) noexcept
        : m_Size(size)
    {
		std::memset(m_Values, 0, sizeof(T) * m_Size);
    }

	~TranspositionTable() noexcept
    {
		delete[] m_Values;
    }

    T get(const U64 key) const noexcept
    {
        std::shared_lock lock(m_Mutex);
        return m_Values[key % m_Size];
    }

    void insert(const T &value) noexcept
    {
        std::unique_lock lock(m_Mutex);
        m_Values[value.key % m_Size] = value;
    }

	void clear() noexcept
	{
		std::unique_lock lock(m_Mutex);
		std::memset(m_Values, 0, sizeof(T) * m_Size);
	}
};
