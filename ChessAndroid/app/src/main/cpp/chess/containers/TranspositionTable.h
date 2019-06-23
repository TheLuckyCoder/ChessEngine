#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>

#include "../data/Enums.h"

using U64 = std::uint64_t;

struct SearchCache
{
    U64 key = 0;
    short boardScore = 0, value = 0;
    short ply = 0;
    Flag flag = Flag::EXACT;
};

class TranspositionTable
{
	constexpr static int MUTEX_SIZE = 1000;

    std::size_t m_Size;
	SearchCache *m_Values = new SearchCache[m_Size]();
	mutable std::shared_mutex m_Mutexes[MUTEX_SIZE];

public:
    explicit TranspositionTable(const std::size_t sizeMb) noexcept;

	TranspositionTable(const TranspositionTable&) = delete;
	TranspositionTable(TranspositionTable&&) = delete;
	~TranspositionTable() noexcept;

	TranspositionTable &operator=(const TranspositionTable&) = delete;
	TranspositionTable &operator=(TranspositionTable&&) = delete;

	SearchCache operator[](const U64 key) const noexcept;

    void insert(const SearchCache &value) noexcept;
	bool setSize(const std::size_t sizeMb) noexcept(false);
	void clear() noexcept;
};
