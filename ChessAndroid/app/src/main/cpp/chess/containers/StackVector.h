#pragma once

#include <utility>
#include <iterator>
#include <initializer_list>

template<class T, std::size_t N>
class StackVector
{
#if __cplusplus >= 201402L
#define CPP14_CONSTEXPR constexpr
#else
#define CPP14_CONSTEXPR
#endif
public:
	using size_type = std::size_t;
	using value_type = T;
	using pointer = value_type * ;
	using const_pointer = const value_type*;
	using reference = value_type & ;
	using const_reference = const value_type&;

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type * ;
		using reference = value_type & ;

		CPP14_CONSTEXPR iterator() noexcept
			: _ptr(nullptr) {}
		CPP14_CONSTEXPR explicit iterator(const_pointer item) noexcept
			: _ptr(const_cast<pointer>(item)) {}
		CPP14_CONSTEXPR iterator(const iterator &iter) noexcept
			: _ptr(iter._ptr) {}
		~iterator() = default;

		iterator &operator=(const iterator&) = default;

		CPP14_CONSTEXPR void operator+=(const size_type n) noexcept { _ptr += n; }
		CPP14_CONSTEXPR void operator+=(const iterator &other) noexcept { _ptr += other._ptr; }
		CPP14_CONSTEXPR iterator operator+(const size_type n) const noexcept
		{
			iterator temp = *this;
			temp += n;
			return temp;
		}
		CPP14_CONSTEXPR difference_type operator+(const iterator &other) const noexcept { return _ptr + other._ptr; }

		CPP14_CONSTEXPR void operator-=(const size_type n) noexcept { _ptr -= n; }
		CPP14_CONSTEXPR void operator-=(const iterator &other) noexcept { _ptr -= other._ptr; }
		CPP14_CONSTEXPR iterator operator-(const size_type n) const noexcept
		{
			iterator temp = *this;
			temp._ptr -= n;
			return temp;
		}
		CPP14_CONSTEXPR difference_type operator-(const iterator &other) const noexcept { return _ptr - other._ptr; }

		constexpr bool operator==(const iterator &iter) const noexcept { return _ptr == iter._ptr; }
		constexpr bool operator!=(const iterator &iter) const noexcept { return _ptr != iter._ptr; }
		constexpr bool operator<(const iterator &iter) const noexcept { return _ptr < iter._ptr; }
		constexpr bool operator<=(const iterator &iter) const noexcept { return _ptr <= iter._ptr; }
		constexpr bool operator>(const iterator &iter) const noexcept { return _ptr > iter._ptr; }
		constexpr bool operator>=(const iterator &iter) const noexcept { return _ptr >= iter._ptr; }

		CPP14_CONSTEXPR iterator &operator++() noexcept
		{
			++_ptr;
			return *this;
		}
		CPP14_CONSTEXPR iterator operator++(int) noexcept
		{
			iterator temp = *this;
			++_ptr;
			return temp;
		}
		CPP14_CONSTEXPR iterator &operator--() noexcept
		{
			--_ptr;
			return *this;
		}
		CPP14_CONSTEXPR iterator operator--(int) noexcept
		{
			iterator temp = *this;
			--_ptr;
			return temp;
		}

		CPP14_CONSTEXPR reference operator[](const difference_type n) noexcept { return *(_ptr + n); }
		constexpr reference operator*() const noexcept { return *_ptr; }
		constexpr pointer operator->() const noexcept { return _ptr; }

	private:
		pointer _ptr;
	};

	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<iterator>;

	CPP14_CONSTEXPR StackVector() noexcept
		: _size(0) {}

	CPP14_CONSTEXPR explicit StackVector(const size_type size) noexcept
		: _size(_size = size > N ? N : size) {}

	CPP14_CONSTEXPR StackVector(std::initializer_list<T> list) noexcept
		: _size(_size = list.size() > N ? N : list.size())
	{
		std::copy(list.begin(), list.begin() + _size, begin());
	}
	CPP14_CONSTEXPR StackVector(const StackVector&) noexcept = default;
	CPP14_CONSTEXPR StackVector(StackVector&&) noexcept = default;

	~StackVector()
	{
		destroyAll(begin(), end());
	}

	CPP14_CONSTEXPR StackVector &operator=(std::initializer_list<T> list) noexcept
	{
		_size = list.size() > N ? N : list.size();

		std::copy(list.begin(), list.begin() + _size, begin());

		return *this;
	}

	template<size_type otherN>
	CPP14_CONSTEXPR StackVector &operator=(const StackVector<T, otherN> &other) noexcept
	{
		_size = other.size() > N ? N : other.size();

		std::copy(other.begin(), other.begin() + _size, begin());

		return *this;
	}


	template<size_type otherN>
	CPP14_CONSTEXPR StackVector &operator+=(StackVector<T, otherN> &&other) noexcept
	{
		size_t otherSize = other.size() > N ? N : other.size();

		if (_size + otherSize > N)
			otherSize = N - (_size + otherSize);

		std::move(other.begin(), other.begin() + otherSize, begin() + _size);

		_size += otherSize;

		return *this;
	}

	CPP14_CONSTEXPR StackVector &operator=(const StackVector&) noexcept = default;
	CPP14_CONSTEXPR StackVector &operator=(StackVector&&) noexcept = default;

	// Element Access
	CPP14_CONSTEXPR reference at(size_type pos) noexcept(false)
	{
		if (pos >= _size) throwLengthException();
		return _array[pos];
	}
	CPP14_CONSTEXPR const_reference at(size_type pos) const noexcept(false)
	{
		if (pos >= _size) throwLengthException();
		return _array[pos];
	}

	CPP14_CONSTEXPR reference operator[](size_type pos) noexcept(false)
	{
		if (pos >= _size) throwLengthException();
		return _array[pos];
	}
	CPP14_CONSTEXPR const_reference operator[](size_type pos) const noexcept(false)
	{
		if (pos >= _size) throwLengthException();
		return _array[pos];
	}

	CPP14_CONSTEXPR reference front() noexcept { return _array[0]; }
	constexpr const_reference front() const noexcept { return _array[0]; }

	CPP14_CONSTEXPR reference back() noexcept { return _array[_size - 1]; }
	constexpr const_reference back() const noexcept { return _array[_size - 1]; }

	CPP14_CONSTEXPR pointer data() noexcept { return _array; }
	constexpr const_pointer data() const noexcept { return _array; }

	// Iterators
	CPP14_CONSTEXPR iterator begin() noexcept { return iterator(_array); }
	constexpr const_iterator begin() const noexcept { return const_iterator(_array); }

	CPP14_CONSTEXPR iterator end() noexcept { return iterator(_array + _size); }
	constexpr const_iterator end() const noexcept { return const_iterator(_array + _size); }

	CPP14_CONSTEXPR reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

	CPP14_CONSTEXPR reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

	// Capacity
	constexpr bool empty() const noexcept { return _size == 0; }
	constexpr bool full() const noexcept { return _size == N; }
	constexpr size_type size() const noexcept { return _size; }
	static constexpr size_type capacity() noexcept { return N; }

	// Modifiers
	CPP14_CONSTEXPR void clear() noexcept
	{
		destroyAll(begin(), end());
		_size = 0;
	}

	template<class... Args >
	CPP14_CONSTEXPR reference emplace(const size_type pos, Args&&... args) noexcept(false)
	{
		if (++_size > N) throwLengthException();
		std::move(_array + pos, _array + _size - 1, _array + pos + 1);

		reference ref = _array[_size - 1];
		ref.~T();

		new (&ref) T(std::forward<Args>(args)...);

		return ref;
	}

	CPP14_CONSTEXPR iterator erase(iterator pos) noexcept
	{
		if (std::is_destructible<T>::value)
			(*pos).~T();
		std::move(pos + 1, end(), pos);
		--_size;

		return pos;
	}

	CPP14_CONSTEXPR iterator erase(iterator first, iterator last) noexcept
	{
		destroyAll(first, last);
		std::move(last, end(), first);

		_size -= last - first;

		return last;
	}

	CPP14_CONSTEXPR void push_back(T &&value) noexcept(false)
	{
		if (++_size > N) throwLengthException();
		_array[_size - 1] = std::move(value);
	}

	CPP14_CONSTEXPR void push_back(const T &value) noexcept(false)
	{
		if (++_size > N) throwLengthException();
		_array[_size - 1] = value;
	}

	template<class... Args >
	CPP14_CONSTEXPR reference emplace_back(Args&&... args) noexcept(false)
	{
		if (++_size > N) throwLengthException();

		reference ref = _array[_size - 1];
		new (&ref) T(std::forward<Args>(args)...);

		return ref;
	}

	CPP14_CONSTEXPR void pop_front() noexcept
	{
		if (_size > 0)
			erase(begin());
	}

	CPP14_CONSTEXPR void pop_back() noexcept
	{
		if (_size > 0)
		{
			--_size;
			_array[_size].~T();
		}
	}

private:
	union
	{
		bool _hidden[N * sizeof(T)];
		value_type _array[N];
	};
	size_type _size;

	CPP14_CONSTEXPR void destroyAll(iterator first, iterator last)
	{
		if constexpr (!std::is_trivially_destructible_v<T>)
		{
			while (first != last)
			{
				first->~T();
				++first;
			}
		}
	}

	static CPP14_CONSTEXPR void throwLengthException() noexcept(false)
	{
	}
};
