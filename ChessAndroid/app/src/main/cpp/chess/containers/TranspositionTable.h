#pragma once

#include <shared_mutex>

#include "../data/Move.h"

struct SearchEntry
{
	enum class Flag : byte
	{
		EXACT,
		ALPHA,
		BETA
	};
	
    U64 key{};
    Move move;
    byte depth{};
    Flag flag = Flag::EXACT;
    unsigned short age{};
};

class TranspositionTable
{
	constexpr static int MUTEX_COUNT = 1024;

public:
    explicit TranspositionTable(std::size_t sizeMb) noexcept;

	TranspositionTable(const TranspositionTable&) = delete;
	TranspositionTable(TranspositionTable&&) = delete;
	~TranspositionTable() noexcept;

	TranspositionTable &operator=(const TranspositionTable&) = delete;
	TranspositionTable &operator=(TranspositionTable&&) = delete;

	SearchEntry operator[](U64 key) const noexcept;

    void insert(const SearchEntry &value) const noexcept;
	bool setSize(std::size_t sizeMb) noexcept(false);
	void incrementAge() noexcept;
	byte currentAge() const noexcept;
	void clear() noexcept;

private:
    std::size_t _size;
    byte _currentAge{};
	SearchEntry *_entries = new SearchEntry[_size]();
	mutable std::shared_mutex _mutexes[MUTEX_COUNT];
};
