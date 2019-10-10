#pragma once

#include <utility>
#include <iterator>
#include <initializer_list>

template<class T, std::size_t N>
class StackVector
{
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

		constexpr iterator() noexcept
			: _ptr(nullptr) {}
		constexpr explicit iterator(const_pointer item) noexcept
			: _ptr(const_cast<pointer>(item)) {}
		constexpr iterator(const iterator &iter) noexcept
			: _ptr(iter._ptr) {}
		~iterator() = default;

		iterator &operator=(const iterator&) = default;

		constexpr void operator+=(const size_type n) noexcept { _ptr += n; }
		constexpr void operator+=(const iterator &other) noexcept { _ptr += other._ptr; }
		constexpr iterator operator+(const size_type n) const noexcept
		{
			iterator temp = *this;
			temp += n;
			return temp;
		}
		constexpr difference_type operator+(const iterator &other) const noexcept { return _ptr + other._ptr; }

		constexpr void operator-=(const size_type n) noexcept { _ptr -= n; }
		constexpr void operator-=(const iterator &other) noexcept { _ptr -= other._ptr; }
		constexpr iterator operator-(const size_type n) const noexcept
		{
			iterator temp = *this;
			temp._ptr -= n;
			return temp;
		}
		constexpr difference_type operator-(const iterator &other) const noexcept { return _ptr - other._ptr; }

		constexpr bool operator==(const iterator &iter) const noexcept { return _ptr == iter._ptr; }
		constexpr bool operator!=(const iterator &iter) const noexcept { return _ptr != iter._ptr; }
		constexpr bool operator<(const iterator &iter) const noexcept { return _ptr < iter._ptr; }
		constexpr bool operator<=(const iterator &iter) const noexcept { return _ptr <= iter._ptr; }
		constexpr bool operator>(const iterator &iter) const noexcept { return _ptr > iter._ptr; }
		constexpr bool operator>=(const iterator &iter) const noexcept { return _ptr >= iter._ptr; }

		constexpr iterator &operator++() noexcept
		{
			++_ptr;
			return *this;
		}
		constexpr iterator operator++(int) noexcept
		{
			iterator temp = *this;
			++_ptr;
			return temp;
		}
		constexpr iterator &operator--() noexcept
		{
			--_ptr;
			return *this;
		}
		constexpr iterator operator--(int) noexcept
		{
			iterator temp = *this;
			--_ptr;
			return temp;
		}

		constexpr reference operator[](const difference_type n) noexcept { return *(_ptr + n); }
		constexpr reference operator*() const noexcept { return *_ptr; }
		constexpr pointer operator->() const noexcept { return _ptr; }

	private:
		pointer _ptr;
	};

	using const_iterator = iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<iterator>;

	constexpr StackVector() noexcept
		: _size(0) {}

	constexpr explicit StackVector(const size_type size) noexcept
		: _size(_size = size > N ? N : size) {}

	constexpr StackVector(std::initializer_list<T> list) noexcept
		: _size(_size = list.size() > N ? N : list.size())
	{
		std::copy(list.begin(), list.begin() + _size, begin());
	}
	constexpr StackVector(const StackVector&) noexcept = default;
	constexpr StackVector(StackVector&&) noexcept = default;

	~StackVector()
	{
		destroyAll(begin(), end());
	}

	constexpr StackVector &operator=(std::initializer_list<T> list) noexcept
	{
		_size = list.size() > N ? N : list.size();

		std::copy(list.begin(), list.begin() + _size, begin());

		return *this;
	}

	template<size_type otherN>
	constexpr StackVector &operator=(const StackVector<T, otherN> &other) noexcept
	{
		_size = other.size() > N ? N : other.size();

		std::copy(other.begin(), other.begin() + _size, begin());

		return *this;
	}


	template<size_type otherN>
	constexpr StackVector &operator+=(StackVector<T, otherN> &&other) noexcept
	{
		size_t otherSize = other.size() > N ? N : other.size();

		if (_size + otherSize > N)
			otherSize = N - (_size + otherSize);

		std::move(other.begin(), other.begin() + otherSize, begin() + _size);

		_size += otherSize;

		return *this;
	}

	constexpr StackVector &operator=(const StackVector&) noexcept = default;
	constexpr StackVector &operator=(StackVector&&) noexcept = default;

	// Element Access
	constexpr reference at(size_type pos) noexcept(false)
	{
		if (pos >= _size) throwLengthException();
		return _array[pos];
	}
	constexpr const_reference at(size_type pos) const noexcept(false)
	{
		if (pos >= _size) throwLengthException();
		return _array[pos];
	}

	constexpr reference operator[](size_type pos) noexcept(false)
	{
		return _array[pos];
	}
	constexpr const_reference operator[](size_type pos) const noexcept(false)
	{
		return _array[pos];
	}

	constexpr reference front() noexcept { return _array[0]; }
	constexpr const_reference front() const noexcept { return _array[0]; }

	constexpr reference back() noexcept { return _array[_size - 1]; }
	constexpr const_reference back() const noexcept { return _array[_size - 1]; }

	constexpr pointer data() noexcept { return _array; }
	constexpr const_pointer data() const noexcept { return _array; }

	// Iterators
	constexpr iterator begin() noexcept { return iterator(_array); }
	constexpr const_iterator begin() const noexcept { return const_iterator(_array); }

	constexpr iterator end() noexcept { return iterator(_array + _size); }
	constexpr const_iterator end() const noexcept { return const_iterator(_array + _size); }

	constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

	constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

	// Capacity
	constexpr bool empty() const noexcept { return _size == 0; }
	constexpr bool full() const noexcept { return _size == N; }
	constexpr size_type size() const noexcept { return _size; }
	static constexpr size_type capacity() noexcept { return N; }

	// Modifiers
	constexpr void clear() noexcept
	{
		destroyAll(begin(), end());
		_size = 0;
	}

	template<class... Args >
	constexpr reference emplace(const size_type pos, Args&&... args) noexcept(false)
	{
		if (++_size > N) throwLengthException();
		std::move(_array + pos, _array + _size - 1, _array + pos + 1);

		reference ref = _array[_size - 1];
		if constexpr (!std::is_trivially_destructible_v<T>)
			ref.~T();

		new (&ref) T(std::forward<Args>(args)...);

		return ref;
	}

	constexpr iterator erase(iterator pos) noexcept
	{
		if constexpr (std::is_destructible<T>::value)
			(*pos).~T();
		std::move(pos + 1, end(), pos);
		--_size;

		return pos;
	}

	constexpr iterator erase(iterator first, iterator last) noexcept
	{
		destroyAll(first, last);
		std::move(last, end(), first);

		_size -= last - first;

		return last;
	}

	constexpr void push_back(T &&value) noexcept(false)
	{
		if (++_size > N) throwLengthException();
		_array[_size - 1] = std::move(value);
	}

	constexpr void push_back(const T &value) noexcept(false)
	{
		if (++_size > N) throwLengthException();
		_array[_size - 1] = value;
	}

	template<class... Args >
	constexpr reference emplace_back(Args&&... args) noexcept(false)
	{
		if (++_size > N) throwLengthException();

		reference ref = _array[_size - 1];
		new (&ref) T(std::forward<Args>(args)...);

		return ref;
	}

	constexpr void pop_front() noexcept
	{
		if (_size > 0)
			erase(begin());
	}

	constexpr void pop_back() noexcept
	{
		if (_size > 0)
		{
			--_size;
			if constexpr (!std::is_trivially_destructible_v<T>)
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

	constexpr void destroyAll(iterator first, iterator last)
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

	static constexpr void throwLengthException() noexcept(false)
	{
	}
};
