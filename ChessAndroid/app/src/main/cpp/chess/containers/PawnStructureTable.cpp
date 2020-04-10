#include "PawnStructureTable.h"

PawnStructureTable::PawnStructureTable(const std::size_t sizeMb) noexcept
	: _size((sizeMb << 20u) / sizeof(PawnStructureEntry)) {}

PawnStructureTable::~PawnStructureTable() noexcept
{
	delete[] _entries;
}

PawnStructureEntry PawnStructureTable::operator[](const U64 key) const noexcept
{
	return _entries[key % _size];
}

void PawnStructureTable::insert(const PawnStructureEntry &value) const noexcept
{
	_entries[value.pawns % _size] = value;
}

bool PawnStructureTable::setSize(const std::size_t sizeMb) noexcept(false)
{
	const auto newSize = (sizeMb << 20u) / sizeof(PawnStructureEntry);

	if (newSize == 0 || _size == newSize) return false;

	_size = newSize;
	delete[] _entries;
	_entries = new PawnStructureEntry[_size]();

	return true;
}

void PawnStructureTable::clear() const noexcept
{
	std::memset(_entries, 0, sizeof(PawnStructureEntry) * _size);
}