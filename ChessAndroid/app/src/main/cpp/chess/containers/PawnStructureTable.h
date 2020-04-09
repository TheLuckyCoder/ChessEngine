#pragma once

#include <mutex>

#include "../data/Defs.h"
#include "../data/Score.h"

struct PawnStructureEntry
{
    U64 pawns{};
    Score score;
};

class PawnStructureTable
{
	static constexpr int MUTEX_COUNT = 128;

public:
    explicit PawnStructureTable(std::size_t sizeMb) noexcept;

	PawnStructureTable(const PawnStructureTable&) = delete;
	PawnStructureTable(PawnStructureTable&&) = delete;
	~PawnStructureTable() noexcept;

	PawnStructureTable &operator=(const PawnStructureTable&) = delete;
	PawnStructureTable &operator=(PawnStructureTable&&) = delete;

	PawnStructureEntry operator[](U64 key) const noexcept;

    void insert(const PawnStructureEntry &value) const noexcept;
	bool setSize(std::size_t sizeMb) noexcept(false);
	void clear() const noexcept;

private:
    std::size_t _size;
	PawnStructureEntry *_entries = new PawnStructureEntry[_size]{};
	mutable std::mutex _mutexes[MUTEX_COUNT];
};

