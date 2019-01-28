#pragma once

#include "MemoryPool.h"

template <typename T>
class PoolAllocator : public std::allocator<T>
{
public:
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using const_pointer = const T*;

	SmallPool *pool = getLocalThreadPool().create();

	PoolAllocator() noexcept
		: std::allocator<T>() {}
	PoolAllocator(const PoolAllocator &other) noexcept
		: std::allocator<T>(other) {}
	template <class U>
	PoolAllocator(const PoolAllocator<U> &other) noexcept
		: std::allocator<T>(other) {}
	~PoolAllocator()
	{
		getLocalThreadPool().destroy(pool);
	}

	template<typename U>
	struct rebind
	{
		typedef PoolAllocator<U> other;
	};

	pointer allocate(const size_type n, const void *hint = nullptr)
	{
		return static_cast<pointer>(pool->allocate(getSizeOfN(n)));
	}

	void deallocate(const pointer p, const size_type n)
	{
		return pool->free(p, getSizeOfN(n));
	}

private:

	constexpr static size_t getSizeOfN(const size_type n)
	{
		constexpr size_t maxPossible = static_cast<size_t>(-1) / sizeof(T);
		size_t result = n * sizeof(T);
		if (maxPossible < n)
		{	// multiply overflow, try allocating all of memory and assume the
			// allocation function will throw bad_alloc
			result = static_cast<size_t>(-1);
		}

		return result;
	}
};
