#pragma once

#include <vector>

template<typename T, typename Hasher = std::hash<T>>
class HashSet
{
	class Node
	{
		T key;
		Node *next = nullptr;

	public:
		explicit Node(T key)
			: key(std::move(key)) {}

		T &getKey() { return key; }
		const T &getKey() const { return key; }

		Node *getNext() const { return next; }

		void setNext(Node *next) { Node::next = next; }
	};

	const size_t _bucketsCount;
	Node **_buckets;
	std::vector<Node> _elements;
	Hasher _hasher;

public:
	explicit HashSet(const size_t elements)
		: _bucketsCount(elements / 2 + 1)
	{
		_buckets = new Node *[_bucketsCount]();
		memset(_buckets, 0, sizeof(_buckets) * _bucketsCount);
		_elements.reserve(elements);
	}

	~HashSet()
	{
		delete[] _buckets;
	}

	bool exists(const T &key) const
	{
		const auto hash = _hasher(key) % _bucketsCount;
		Node *bucket = _buckets[hash];

		while (bucket) {
			if (bucket->getKey() == key)
				return true;
			bucket = bucket->getNext();
		}
		return false;
	}

	void insert(const T &key)
	{
		const auto hash = _hasher(key) % _bucketsCount;

		Node *prevBucket = nullptr;
		Node *bucket = _buckets[hash];

		while (bucket && bucket->getKey() != key)
		{
			prevBucket = bucket;
			bucket = bucket->getNext();
		}

		if (!bucket)
		{
			bucket = &_elements.emplace_back(key);
			if (prevBucket)
				prevBucket->setNext(bucket);
			else
				_buckets[hash] = bucket; // insert as first in bucket
		}
	}

	Hasher &hashFunction() const
	{
		return _hasher;
	}

	void clear()
	{
		memset(_buckets, nullptr, sizeof(_buckets) * _bucketsCount);
		_elements.clear();
	}

};
