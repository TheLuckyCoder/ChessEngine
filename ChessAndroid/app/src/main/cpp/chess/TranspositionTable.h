#pragma once

#include <array>
#include <optional>
#include <shared_mutex>

#include "Move.h"

class SearchEntry
{
	friend class TranspositionTable;

public:
	enum class Bound : u8
	{
		NONE = 0,
		EXACT = 0b01,
		ALPHA = 0b10,
		BETA = 0b11
	};

	SearchEntry() = default;

	constexpr SearchEntry(const u64 key, const int depth, const Move move, const bool qSearch, const Bound bound)
		: _key16(key >> 48u), _moveFromTo(move.getFromToBits()), _value(move.getScore()), _depth8(depth),
		  generation((qSearch << 7u) | (u8(bound) << 5u)) {}

	constexpr u16 key() const noexcept { return _key16; }

	constexpr Move move() const noexcept { return Move{ _moveFromTo, i32(_value) }; }

	constexpr i32 depth() const noexcept { return i32(_depth8); }

	constexpr bool qSearch() const noexcept { return generation & QSEARCH_BITS; }

	constexpr Bound bound() const noexcept
	{
		return static_cast<Bound>((generation & BOUND_BITS) >> 5u);
	}

	constexpr u8 age() const noexcept { return generation & AGE_BITS; }

	constexpr void setAge(const u8 newAge) noexcept
	{
		generation = (generation & ~AGE_BITS) | (newAge & AGE_BITS);
	}

private:
	u16 _key16{};
	u16 _moveFromTo{};
	i16 _value{};
	i8 _depth8{};

	/**
	 * 0b0001'1111 - age
	 * 0b0110'0000 - Bound
	 * 0b1000'0000 - qSearch
	 */
	u8 generation{};

	static constexpr u8 AGE_BITS = 0b0001'1111;
	static constexpr u8 BOUND_BITS = 0b0110'0000;
	static constexpr u8 QSEARCH_BITS = 0b1000'0000;
};

class TranspositionTable
{
	static constexpr usize MUTEX_COUNT = 512;
	static constexpr usize CLUSTER_SIZE = 4;

	struct Cluster
	{
		std::array<SearchEntry, CLUSTER_SIZE> entries;
	};

	static_assert(sizeof(Cluster) == 32, "Wrong Cluster Size");

public:
	explicit TranspositionTable(usize sizeMb);

	TranspositionTable(const TranspositionTable &) = delete;
	TranspositionTable(TranspositionTable &&) = delete;
	~TranspositionTable();

	TranspositionTable &operator=(const TranspositionTable &) = delete;
	TranspositionTable &operator=(TranspositionTable &&) = delete;

	void prefetch(u64 zKey) const noexcept;
	std::optional<SearchEntry> probe(u64 zKey) const noexcept;

	void insert(u64 zKey, SearchEntry entry) noexcept;
	bool setSize(usize sizeMb);
	void update() noexcept;
	u8 currentAge() const noexcept;
	void clear() noexcept;

private:
	usize _size{};
	u64 _hashMask{};
	u8 _currentAge{};
	Cluster *_clusters = nullptr;
	mutable std::array<std::shared_mutex, MUTEX_COUNT> _mutexes{};
};
