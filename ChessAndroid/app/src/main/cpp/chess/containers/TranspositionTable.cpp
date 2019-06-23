#include "TranspositionTable.h"

TranspositionTable::TranspositionTable(const std::size_t sizeMb) noexcept
	: m_Size((sizeMb << 20u) / sizeof(SearchCache)) {}

TranspositionTable::~TranspositionTable() noexcept
{
	delete[] m_Values;
}

SearchCache TranspositionTable::operator[](const U64 key) const noexcept
{
	const auto index = key % m_Size;
	std::shared_lock lock(m_Mutexes[index % MUTEX_SIZE]);
	return m_Values[index];
}

void TranspositionTable::insert(const SearchCache &value) noexcept
{
	const auto index = value.key % m_Size;
	std::lock_guard lock(m_Mutexes[index % MUTEX_SIZE]);

	SearchCache &ref = m_Values[index];
	if (ref.ply <= value.ply)
		ref = value;
}

bool TranspositionTable::setSize(const std::size_t sizeMb) noexcept(false)
{
	const auto newSize = (sizeMb << 20u) / sizeof(SearchCache);

	if (newSize == 0 || m_Size == newSize) return false;

	m_Size = newSize;
	delete[] m_Values;
	m_Values = new SearchCache[m_Size]();

	return true;
}

void TranspositionTable::clear() noexcept
{
	std::memset(m_Values, 0, sizeof(SearchCache) * m_Size);
}
