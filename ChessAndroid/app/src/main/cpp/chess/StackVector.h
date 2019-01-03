#pragma once

#include <utility>
#include <stdexcept>

template<class T, std::size_t N>
class StackVector
{
#define CPP17_CONSTEXPR
#if __cplusplus == 201703L || _MSC_VER >= 1300
#define CPP17
#undef CPP17_CONSTEXPR
#define CPP17_CONSTEXPR constexpr
#endif
public:
	typedef std::size_t size_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = int;
		using pointer = T * ;
		using reference = T & ;

		iterator() : ptr(nullptr) {};
		explicit iterator(const_pointer item) : ptr(const_cast<pointer>(item)) {}
		iterator(const iterator &iter) : ptr(iter.ptr) {}

		iterator &operator=(const iterator&) = default;
		~iterator() = default;

		constexpr iterator operator+(size_type add) const
		{
			return iterator(ptr + add);
		}
		constexpr iterator operator-(size_type sub) const
		{
			return iterator(ptr - sub);
		}

		constexpr bool operator==(const iterator &iter) const
		{
			return ptr == iter.ptr;
		}
		constexpr bool operator!=(const iterator &iter) const noexcept
		{
			return ptr != iter.ptr;
		}
		constexpr bool operator<(const iterator &iter) const noexcept
		{
			return ptr < iter.ptr;
		}
		constexpr bool operator<=(const iterator &iter) const noexcept
		{
			return ptr <= iter.ptr;
		}
		constexpr bool operator>(const iterator &iter) const noexcept
		{
			return ptr > iter.ptr;
		}
		constexpr bool operator>=(const iterator &iter) const noexcept
		{
			return ptr >= iter.ptr;
		}

		iterator &operator++() noexcept
		{
			++ptr;
			return *this;
		}
		iterator operator++(int) noexcept
		{
			iterator temp = *this;
			++ptr;
			return temp;
		}
		iterator &operator--() noexcept
		{
			--ptr;
			return *this;
		}
		iterator operator--(int) noexcept
		{
			iterator temp = *this;
			--ptr;
			return temp;
		}
		reference operator*() const
		{
			return *ptr;
		}
		pointer operator->() const
		{
			return ptr;
		}

	private:
		pointer ptr;
	};

	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<iterator>;

	StackVector() noexcept : m_Size(0) {}
	explicit StackVector(size_type size) : m_Size(size)
	{
		if (m_Size > N) throwLengthException();

	}

	// Element Access
	T &at(size_type pos)
	{
		if (pos >= m_Size) throwLengthException();
		return m_Array[pos];
	}
	const T &at(size_type pos) const
	{
		if (pos >= m_Size) throwLengthException();
		return m_Array[pos];
	}

	T &operator[](size_type pos)
	{
		if (pos >= m_Size) throwLengthException();
		return m_Array[pos];
	}
	const T &operator[](size_type pos) const
	{
		if (pos >= m_Size) throwLengthException();
		return m_Array[pos];
	}

	reference front() { return m_Array[0]; }
	constexpr const_reference front() const noexcept { return m_Array[0]; }

	reference back() { return m_Array[m_Size - 1]; }
	constexpr const_reference back() const noexcept { return m_Array[m_Size - 1]; }

	pointer data() noexcept { return m_Array; }
	const_pointer data() const noexcept { return m_Array; }

	// Iterators
	iterator begin() noexcept { return iterator(m_Array); }
	const_iterator begin() const noexcept { return const_iterator(m_Array); }

	iterator end() noexcept { return iterator(m_Array + m_Size); }
	const_iterator end() const noexcept { return const_iterator(m_Array + m_Size); }

	reverse_iterator rbegin() noexcept { return reverse_iterator(m_Array + m_Size - 1); }
	const_reverse_iterator rbegin() const noexcept { return const_iterator(m_Array + m_Size - 1); }

	reverse_iterator rend() noexcept { return reverse_iterator(m_Array - 1); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(m_Array - 1); }

	// Capacity
	constexpr bool empty() const noexcept { return m_Size == 0; }
	constexpr size_type size() const noexcept { return m_Size; }
	constexpr size_type capacity() const noexcept { return N; }

	// Modifiers
	void clear()
	{
		m_Size = 0;
	}

	//insert

	template<class... Args >
	reference emplace(size_type pos, Args&&... args)
	{
		if (++m_Size > N) throwLengthException();
		std::move(m_Array + pos, m_Array + m_Size - 1, m_Array + pos + 1);
		return m_Array[pos] = T(std::forward<T>(args)...);
	}

	iterator erase(iterator pos)
	{
		std::move(pos + 1, end(), pos);
		--m_Size;

		return pos;
	}

	iterator erase(iterator first, iterator last)
	{
		std::move(last, end(), first);
		int elements = 0;
		while (first != last)
		{
			++first;
			++elements;
		}
		m_Size -= elements;

		return last;
	}

	void push_back(T &&value)
	{
		if (++m_Size > N) throwLengthException();
		m_Array[m_Size - 1] = std::move(value);
	}

	void push_back(const T &value)
	{
		if (++m_Size > N) throwLengthException();
		m_Array[m_Size - 1] = value;
	}

	template<class... Args >
	reference emplace_back(Args&&... args)
	{
		if (++m_Size > N) throwLengthException();
		m_Array[m_Size - 1] = T(std::forward<Args>(args)...);
		return back();
	}

	void pop_front()
	{
		erase(begin());
	}

	void pop_back()
	{
		if (m_Size > 0) --m_Size;
	}

#ifdef CPP17
	template<size_type otherN>
	StackVector &operator=(const StackVector<T, otherN> &other)
	{
		if constexpr (otherN > N) throwLengthException();

		std::copy(other.begin(), other.end(), begin());
		m_Size = other.size();

		return *this;
	}
#endif

private:
	value_type m_Array[N];
	size_type m_Size;

	CPP17_CONSTEXPR static void throwLengthException()
	{
		throw std::out_of_range("Size of the Vector can not be larger than the max allocated size");
	}
};
