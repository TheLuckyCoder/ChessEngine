#pragma once

#include <mutex>
#include <shared_mutex>
#include <vector>

template <typename K, typename V>
class HashNode
{
	// key-value pair
	K key;
	V value;
	// next bucket with the same hash
	HashNode *next;

public:
	HashNode(K key, V value) :
		key(std::move(key)), value(std::move(value)), next(nullptr) {}

	K &getKey() { return key; }
	const K &getKey() const { return key; }

	V &getValue() { return value; }
	const V &getValue() const { return value; }

	void setValue(V &&value)
	{
		HashNode::value = std::move(value);
	}

	HashNode *getNext() const { return next; }

	void setNext(HashNode *next)
	{
		HashNode::next = next;
	}

};

template<typename Val>
class HashTable
{
	using Key = std::uint64_t;

	mutable std::shared_mutex _mutex;

	HashNode<Key, Val> **_pointerTable;
	std::size_t _tableSize;
	std::vector<HashNode<Key, Val>> _values;

public:
	explicit HashTable(const std::size_t tableSize)
		: _tableSize(tableSize / 4)
	{
		_pointerTable = new HashNode<Key, Val> *[_tableSize]();
		_values.reserve(tableSize);
	}

	~HashTable()
	{
		delete[] _pointerTable;
	}

	bool get(const Key &key, Val &value) const
	{
		std::shared_lock lock(_mutex);

		auto *entry = _pointerTable[key % _tableSize];

		while (entry) {
			if (entry->getKey() == key) {
				value = entry->getValue();
				return true;
			}
			entry = entry->getNext();
		}
		return false;
	}

	void insert(const Key &key, Val value)
	{
		const size_t hash = key % _tableSize;
		std::unique_lock lock(_mutex);

		HashNode<Key, Val> *prev = nullptr;
		auto *entry = _pointerTable[hash];

		while (entry && entry->getKey() != key)
		{
			prev = entry;
			entry = entry->getNext();
		}

		if (entry)
			entry->setValue(std::forward<Val>(value)); // update the value
		else
		{
			entry = &_values.emplace_back(key, std::forward<Val>(value));
			if (prev)
				prev->setNext(entry);
			else
				_pointerTable[hash] = entry; // insert as first bucket
		}
	}

	void checkSize()
	{
		std::unique_lock lock(_mutex);
		if (_tableSize - _values.size() < 2000000u)
			clear();
	}

	void clear()
	{
		std::memset(_pointerTable, 0, sizeof _pointerTable);
		_values.clear();
	}
};
