#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>

using U64 = std::uint64_t;

struct SearchCache
{
    U64 key = 0;
    int score = 0, bestMove = 0;
    short depth = 0;
};

template <class T>
class TranspositionTable
{
    std::size_t m_Size;
	T *m_Values = new T[m_Size]();
    mutable std::shared_mutex m_Mutex;

public:
    explicit TranspositionTable(const std::size_t sizeMb) noexcept
        : m_Size((sizeMb << 20u) / sizeof(T)) {}

	TranspositionTable(const TranspositionTable&) = delete;
	TranspositionTable &operator=(const TranspositionTable&) = delete;
	TranspositionTable(TranspositionTable&&) = delete;
	TranspositionTable &operator=(TranspositionTable&&) = delete;

	~TranspositionTable() noexcept
    {
		delete[] m_Values;
    }

    T operator[](const U64 key) const noexcept
    {
        std::shared_lock lock(m_Mutex);
        return m_Values[key % m_Size];
    }

    void insert(const T &value) noexcept
    {
        std::lock_guard lock(m_Mutex);

		auto &ref = m_Values[value.key % m_Size];
		if (ref.depth <= value.depth)
			ref = value;
    }

	void setSize(const std::size_t sizeMb) noexcept(false)
	{
		const auto newSize = (sizeMb << 20u) / sizeof(T);

		if (newSize == 0 || m_Size == newSize) return;

		std::lock_guard lock(m_Mutex);
		m_Size = newSize;
		delete[] m_Values;
		m_Values = new T[m_Size]();
	}

	void clear() noexcept
	{
		std::lock_guard lock(m_Mutex);
		std::memset(m_Values, 0, sizeof(T) * m_Size);
	}
};
