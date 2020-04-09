#include "PawnStructureTable.h"

PawnStructureTable::PawnStructureTable(const std::size_t sizeMb) noexcept
	: _size((sizeMb << 20u) / sizeof(PawnStructureEntry)) {}

PawnStructureTable::~PawnStructureTable() noexcept
{
	delete[] _entries;
}

PawnStructureEntry PawnStructureTable::operator[](const U64 key) const noexcept
{
	const auto index = key % _size;
	std::lock_guard lock{ _mutexes[index % MUTEX_COUNT] };
	return _entries[index];
}

void PawnStructureTable::insert(const PawnStructureEntry &value) const noexcept
{
	const auto index = value.pawns % _size;
	std::lock_guard lock{ _mutexes[index % MUTEX_COUNT] };

	_entries[index] = value;
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