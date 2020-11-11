#include "TranspositionTable.h"

#include <cstring>
#include <iostream>

static constexpr u64 MB = 1ull << 20;

TranspositionTable::TranspositionTable(const usize sizeMb)
{
	setSize(sizeMb);
}

TranspositionTable::~TranspositionTable()
{
	delete[] _entries;
}

void TranspositionTable::prefetch(const u64 zKey) const noexcept
{
#if defined(__has_builtin) && __has_builtin(__builtin_prefetch)
	__builtin_prefetch(&_entries[zKey & _hashMask]);
#endif
}

std::optional<SearchEntry> TranspositionTable::probe(const u64 zKey) const noexcept
{
	const u16 key16 = zKey >> 48u;
	const auto index = zKey & _hashMask;

	assert(index < _size);
	auto &clusterEntries = _entries[index].entries;

	std::shared_lock lock{ _mutexes[index % MUTEX_COUNT] };
	for (usize i = 0; i < CLUSTER_SIZE; ++i)
	{
		auto &entry = clusterEntries[i];
		if (key16 == entry.key() && currentAge() == entry.age())
			return entry;
	}

	return {};
}

void TranspositionTable::insert(const u64 zKey, SearchEntry entry) noexcept
{
	const auto key = entry.key();
	const auto index = zKey & _hashMask;

	assert(index < _size);
	auto &clusterEntries = _entries[index].entries;
	auto *toReplace = &clusterEntries.front();

	std::lock_guard lock{ _mutexes[index % MUTEX_COUNT] };

	usize i{};
	for (; i < CLUSTER_SIZE && key != clusterEntries[i].key(); ++i)
	{
		if (!clusterEntries[i].key() || clusterEntries[i].age() != currentAge())
		{
			toReplace = &clusterEntries[i];
			break;
		}

		if (toReplace->depth() > clusterEntries[i].depth())
			toReplace = &clusterEntries[i];
	}

	// Don't overwrite an entry from the same position, unless we have
	// an exact bound or depth that is nearly as good as the old one
	if (toReplace->age() == currentAge()
		&& key == toReplace->key()
		&& entry.bound() != SearchEntry::Bound::EXACT
		&& entry.depth() < toReplace->depth() - 2)
		return;

	entry.setAge(currentAge());
	*toReplace = entry;
}

bool TranspositionTable::setSize(usize sizeMb)
{
	const auto newSize = (sizeMb << 20u) / sizeof(Bucket);

	if (newSize == 0 || _size == newSize) return false;

	_size = newSize;
	delete[] _entries;
	_entries = nullptr;

	while (!_entries && sizeMb)
	{
		const auto bytesSize = sizeof(Bucket) * _size;
		_entries = static_cast<Bucket *>(operator new[](bytesSize, std::nothrow));

		if (_entries)
			clear();
		else
		{
			std::cerr << "Failed to allocate " << sizeMb << "MB for the Transposition Table\n";
			sizeMb /= 2;
			_size = (sizeMb << 20u) / sizeof(Bucket);
		}
	}

	u64 keySize = 16u;
	while ((1ull << keySize) * sizeof(Bucket) <= sizeMb * MB / 2)
		++keySize;

	_hashMask = (1ull << keySize) - 1u;

	return true;
}

void TranspositionTable::incrementAge() noexcept
{
	(++_currentAge) &= SearchEntry::AGE_BITS;
}

u8 TranspositionTable::currentAge() const noexcept
{
	return _currentAge;
}

void TranspositionTable::clear() noexcept
{
	std::memset(_entries, 0, sizeof(Bucket) * _size);
	_currentAge = {};
}
