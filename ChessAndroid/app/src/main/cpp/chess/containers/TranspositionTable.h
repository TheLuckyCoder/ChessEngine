#pragma once

#include <shared_mutex>

#include "../Move.h"

struct SearchEntry
{
	enum class Flag : u8
	{
		NONE,
		EXACT,
		ALPHA,
		BETA
	};
	
    u64 key{};
    Move move;
    short depth{};
    Flag flag = Flag::EXACT;
	bool qSearch{};
    unsigned short age{};
};

class TranspositionTable
{
	static constexpr int MUTEX_COUNT = 2048;

public:
    explicit TranspositionTable(std::size_t sizeMb);

	TranspositionTable(const TranspositionTable&) = delete;
	TranspositionTable(TranspositionTable&&) = delete;
	~TranspositionTable() noexcept;

	TranspositionTable &operator=(const TranspositionTable&) = delete;
	TranspositionTable &operator=(TranspositionTable&&) = delete;

	SearchEntry operator[](u64 key) const noexcept;

    void insert(const SearchEntry &value) noexcept;
	bool setSize(std::size_t sizeMb);
	void incrementAge() noexcept;
	u8 currentAge() const noexcept;
	void clear() noexcept;

private:
    std::size_t _size{};
    u8 _currentAge{};
	SearchEntry *_entries = nullptr;
	mutable std::shared_mutex _mutexes[MUTEX_COUNT];
};
