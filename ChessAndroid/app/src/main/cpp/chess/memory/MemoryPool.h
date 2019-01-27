#pragma once

#include <algorithm>
#include <vector>

class MemoryPool
{
	using MemoryUnit = char;
	using MemoryPtr = MemoryUnit * ;

	struct Piece
	{
		// Piece is a range of memory [begin, end)
		MemoryPtr begin, end;

		Piece(const MemoryPtr start, const MemoryPtr end)
			: begin(start), end(end) {}

		size_t getSize() const { return sizeof(MemoryUnit) * (end - begin); }
	};

	MemoryUnit *buffer;
	std::vector<Piece> available;

	void unifyFrom(const size_t index)
	{
		MemoryPtr lastEnd = available[index].end;
		auto itr = available.begin() + index + 1;

		for (; itr != available.end(); ++itr)
			if (itr->begin == lastEnd)
				lastEnd = itr->end;
			else break;

		available.erase(std::begin(available) + index, itr);
		available.emplace_back(available[index].begin, lastEnd);
	}

	void unifyContiguous()
	{
		std::sort(available.begin(), available.end(),
			[](const Piece &first, const Piece &second) { return first.begin < second.begin; });
		for (size_t i = 0; i < available.size(); ++i)
			unifyFrom(i);
	}

	std::vector<Piece>::iterator findSuitableMemory(const size_t requiredSize)
	{
		// Tries to find a memory piece big enough to hold requiredSize
		// If it is not found, contiguous memory pieces are unified
		// If it is not found again, throws an exception

		for (size_t i = 0; i < 2; ++i)
		{
			for (auto itr = available.begin(); itr != available.end(); ++itr)
				if (itr->getSize() >= requiredSize)
					return itr;
			unifyContiguous();
		}
		throw "Unable to find Suitable Memory";
	}

public:
	explicit MemoryPool(const size_t bufferSize)
	{
		buffer = new MemoryUnit[bufferSize];
		// Add the whole buffer to the available memory vector
		available.emplace_back(&buffer[0], &buffer[bufferSize]);
	}
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool(MemoryPool&&) = delete;
	~MemoryPool()
	{
		delete[] buffer;
	}

	MemoryPool &operator=(const MemoryPool&) = delete;
	MemoryPool &operator=(MemoryPool&&) = delete;

	template<typename T, typename... Args>
	T *create(Args&&... args)
	{
		// Creates and returns a T* allocated with "placement new" on an available piece of the buffer
		// T must be the "real object type" - this method will fail with pointers to bases that store derived instances!

		constexpr auto requiredSize = sizeof(T);
		const auto suitable = findSuitableMemory(requiredSize);

		const MemoryPtr toUse = suitable->begin;
		Piece leftover(toUse + requiredSize, suitable->end);

		available.erase(suitable);
		if (leftover.getSize() > 0)
			available.push_back(leftover);

		return new (toUse) T(std::forward<Args>(args)...);
	}

	template<typename T>
	void destroy(T *object)
	{
		// Destroys a previously allocated object, calling its destructor and reclaiming its memory piece
		if constexpr (std::is_destructible_v<T>)
			object->~T();

		auto objStart(reinterpret_cast<MemoryPtr>(object));
		available.emplace_back(objStart, objStart + sizeof(T));
	}

	template<typename T = void>
	T *allocate(const size_t size)
	{
		const auto suitable = findSuitableMemory(size);

		const MemoryPtr toUse = suitable->begin;
		Piece leftover(toUse + size, suitable->end);

		available.erase(suitable);
		if (leftover.getSize() > 0)
			available.push_back(leftover);

		return reinterpret_cast<T*>(toUse);
	}

	void free(void *ptr, const size_t size)
	{
		auto objStart = reinterpret_cast<MemoryPtr>(ptr);
		available.emplace_back(objStart, objStart + size);
	}

	static MemoryPool &getMemPoolPerThread()
	{
		static thread_local MemoryPool memPool(20971520); // 20 MB
		return memPool;
	}
};
