#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>

using U64 = std::uint64_t;

struct SearchCache
{
    U64 key = 0;
    int boardScore = 0, value = 0;
    short depth = 0;
    Flag flag = Flag::EXACT;
};

template <class T>
class TranspositionTable
{
	constexpr static int MUTEX_SIZE = 1000;

    std::size_t m_Size;
	T *m_Values = new T[m_Size]();
	mutable std::shared_mutex m_Mutexes[MUTEX_SIZE];

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
		const auto index = key % m_Size;
        std::shared_lock lock(m_Mutexes[index % MUTEX_SIZE]);
        return m_Values[index];
    }

    void insert(const T &value) noexcept
    {
		const auto index = value.key % m_Size;
        std::lock_guard lock(m_Mutexes[index % MUTEX_SIZE]);

		auto &ref = m_Values[index];
		if (ref.depth <= value.depth)
			ref = value;
    }

	void setSize(const std::size_t sizeMb) noexcept(false)
	{
		const auto newSize = (sizeMb << 20u) / sizeof(T);

		if (newSize == 0 || m_Size == newSize) return;

		m_Size = newSize;
		delete[] m_Values;
		m_Values = new T[m_Size]();
	}

	void clear() noexcept
	{
		std::memset(m_Values, 0, sizeof(T) * m_Size);
	}
};
