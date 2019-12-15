#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>

#include "../data/Defs.h"

using U64 = std::uint64_t;
using byte = unsigned char;

struct SearchCache
{
	U64 key = 0;
	short boardScore = 0, value = 0;
	short ply = 0;
	Flag flag = Flag::EXACT;
	byte age = 0;
};

class TranspositionTable
{
	constexpr static int MUTEX_COUNT = 1024;

	std::size_t m_Size;
	byte m_CurrentAge{};
	SearchCache *m_Values = new SearchCache[m_Size]();
	mutable std::shared_mutex m_Mutexes[MUTEX_COUNT];

public:
	explicit TranspositionTable(std::size_t sizeMb) noexcept;

	TranspositionTable(const TranspositionTable &) = delete;
	TranspositionTable(TranspositionTable &&) = delete;
	~TranspositionTable() noexcept;

	TranspositionTable &operator=(const TranspositionTable &) = delete;
	TranspositionTable &operator=(TranspositionTable &&) = delete;

	SearchCache operator[](U64 key) const noexcept;

	void insert(const SearchCache &value) noexcept;
	bool setSize(std::size_t sizeMb) noexcept(false);
	void incrementAge() noexcept;
	byte currentAge() const noexcept;
	void clear() noexcept;
};
