#pragma once

#include "../Bits.h"
#include "../Score.h"

struct PawnStructureEntry
{
    Bitboard pawns;
    Score score;
};

class PawnStructureTable
{
public:
    explicit PawnStructureTable(usize sizeMb) noexcept;

	PawnStructureTable(const PawnStructureTable&) = delete;
	PawnStructureTable(PawnStructureTable&&) = delete;
	~PawnStructureTable() noexcept;

	PawnStructureTable &operator=(const PawnStructureTable&) = delete;
	PawnStructureTable &operator=(PawnStructureTable&&) = delete;

	PawnStructureEntry operator[](u64 key) const noexcept;

    void insert(const PawnStructureEntry &value) const noexcept;
	bool setSize(usize sizeMb) noexcept(false);
	void clear() const noexcept;

private:
	usize _size;
	PawnStructureEntry *_entries = new PawnStructureEntry[_size]{};
};
