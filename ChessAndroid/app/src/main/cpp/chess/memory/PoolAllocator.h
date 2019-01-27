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

	PoolAllocator() noexcept : std::allocator<T>() {}
	PoolAllocator(const PoolAllocator &a) noexcept : std::allocator<T>(a) {}
	template <class U>
	PoolAllocator(const PoolAllocator<U> &a) noexcept : std::allocator<T>(a) {}
	~PoolAllocator() = default;

	template<typename U>
	struct rebind
	{
		typedef PoolAllocator<U> other;
	};

	pointer allocate(const size_type n, const void *hint = nullptr)
	{
		//return std::allocator<T>::allocate(n, hint);
		return MemoryPool::getMemPoolPerThread().allocate<T>(n);
	}

	void deallocate(pointer p, size_type n)
	{
		return MemoryPool::getMemPoolPerThread().free(p, n);
	}
};
