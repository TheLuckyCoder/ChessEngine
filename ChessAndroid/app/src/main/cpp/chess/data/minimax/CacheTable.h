#pragma once

#include <mutex>
#include <shared_mutex>

template <typename K, typename V>
class HashNode {
private:
	// key-value pair
	K key;
	V value;
	// next bucket with the same key
	HashNode *next;

public:
	HashNode(K key, V value) :
		key(std::move(key)), value(std::move(value)), next(nullptr) {}

	K getKey() const
	{
		return key;
	}

	V getValue() const
	{
		return value;
	}

	void setValue(V value)
	{
		HashNode::value = std::move(value);
	}

	HashNode *getNext() const
	{
		return next;
	}

	void setNext(HashNode *next)
	{
		HashNode::next = next;
	}

};

class CacheTable
{
private:
	using Key = std::uint64_t;
	using Val = int;

	// hash table
	HashNode<Key, Val> **table;
	mutable std::shared_mutex mutex_;

	constexpr static size_t TABLE_SIZE = 40000000;

public:
	CacheTable()
	{
		table = new HashNode<Key, Val> *[TABLE_SIZE]();
	}

	~CacheTable()
	{
		for (size_t i = 0; i < TABLE_SIZE; ++i) {
			auto *entry = table[i];
			while (entry) {
				auto *prev = entry;
				entry = entry->getNext();
				delete prev;
			}
		}

		delete[] table;
	}

	bool get(const Key &key, Val &value) const
	{
		const size_t hash = key % TABLE_SIZE;
		std::shared_lock lock(mutex_);

		auto *entry = table[hash];

		while (entry) {
			if (entry->getKey() == key) {
				value = entry->getValue();
				return true;
			}
			entry = entry->getNext();
		}
		return false;
	}

	void put(const Key &key, Val value)
	{
		std::unique_lock lock(mutex_);

		HashNode<Key, Val> *prev = nullptr;
		auto *entry = table[key];

		while (entry && entry->getKey() != key)
		{
			prev = entry;
			entry = entry->getNext();
		}

		if (entry)
		{
			// just update the value
			entry->setValue(std::forward<Val>(value));
		}
		else
		{
			entry = new HashNode<Key, Val>(key, std::forward<Val>(value));
			if (prev)
				prev->setNext(entry);
			else
				table[key] = entry; // insert as first bucket
		}
	}

	void clearAll()
	{
		for (size_t i = 0; i < TABLE_SIZE; ++i) {
			auto *entry = table[i];
			while (entry) {
				auto *prev = entry;
				entry = entry->getNext();
				delete prev;
			}
		}
	}

/*private:
	void remove(const Key &key)
	{
		std::unique_lock lock(mutex_);

		HashNode<Key, Val> *prev = nullptr;
		auto *entry = table[key];

		while (entry && entry->getKey() != key) {
			prev = entry;
			entry = entry->getNext();
		}

		if (!entry) return; // key not found

		if (prev)
			prev->setNext(entry->getNext()); // remove first bucket of the list
		else
			table[key] = entry->getNext();

		delete entry;
	}*/

};
