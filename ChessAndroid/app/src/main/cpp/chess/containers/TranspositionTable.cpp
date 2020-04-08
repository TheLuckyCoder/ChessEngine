#include "TranspositionTable.h"

TranspositionTable::TranspositionTable(const std::size_t sizeMb) noexcept
	: _size((sizeMb << 20u) / sizeof(SearchEntry)) {}

TranspositionTable::~TranspositionTable() noexcept
{
	delete[] _entries;
}

SearchEntry TranspositionTable::operator[](const U64 key) const noexcept
{
	const auto index = key % _size;
	std::shared_lock lock(_mutexes[index % MUTEX_COUNT]);
	return _entries[index];
}

void TranspositionTable::insert(const SearchEntry &value) const noexcept
{
	const auto index = value.key % _size;
	std::lock_guard lock(_mutexes[index % MUTEX_COUNT]);

	SearchEntry &ref = _entries[index];
	if (ref.depth <= value.depth || ref.age != _currentAge)
	{
		ref = value;
		ref.age = _currentAge;
	}
}

bool TranspositionTable::setSize(const std::size_t sizeMb) noexcept(false)
{
	const auto newSize = (sizeMb << 20u) / sizeof(SearchEntry);

	if (newSize == 0 || _size == newSize) return false;

	_size = newSize;
	delete[] _entries;
	_entries = new SearchEntry[_size]();
	_currentAge = 0u;

	return true;
}

void TranspositionTable::incrementAge() noexcept
{
	++_currentAge;
}

byte TranspositionTable::currentAge() const noexcept
{
	return _currentAge;
}

void TranspositionTable::clear() noexcept
{
	std::memset(_entries, 0, sizeof(SearchEntry) * _size);
	_currentAge = 0u;
}
