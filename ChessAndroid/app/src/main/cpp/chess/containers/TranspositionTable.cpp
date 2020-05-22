#include "TranspositionTable.h"

#include <cstring>
#include <iostream>

TranspositionTable::TranspositionTable(const std::size_t sizeMb)
{
	setSize(sizeMb);
}

TranspositionTable::~TranspositionTable() noexcept
{
	delete[] _entries;
}

SearchEntry TranspositionTable::operator[](const U64 key) const noexcept
{
	const auto index = key % _size;
	std::shared_lock lock{ _mutexes[index % MUTEX_COUNT] };
	return _entries[index];
}

void TranspositionTable::insert(const SearchEntry &value) noexcept
{
	const auto index = value.key % _size;
	std::lock_guard lock{ _mutexes[index % MUTEX_COUNT] };

	SearchEntry &ref = _entries[index];
	if (ref.depth <= value.depth || ref.age != _currentAge)
	{
		ref = value;
		ref.age = _currentAge;
	}
}

bool TranspositionTable::setSize(std::size_t sizeMb)
{
	const auto newSize = (sizeMb << 20u) / sizeof(SearchEntry);

	if (newSize == 0 || _size == newSize) return false;

	_size = newSize;
	delete[] _entries;
	_entries = nullptr;
	_currentAge = 0u;

	while (!_entries && sizeMb)
	{
		const size_t bytesSize = _size * sizeof(SearchEntry);
		_entries = static_cast<SearchEntry *>(operator new [](bytesSize, std::nothrow));
		
		if (_entries)
			std::memset(_entries, 0, bytesSize);
		else
		{
			std::cerr << "Failed to allocate " << sizeMb << "MB for the Transposition Table\n";
			sizeMb /= 2;
			_size = (sizeMb << 20u) / sizeof(SearchEntry);
		}
	}

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
