#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Range type representing a pair of iterators
template<typename Iter>
class range {
	Iter iter_begin, iter_end;

public:
	range() = default;
	range(Iter a, Iter b)
		: iter_begin(a), iter_end(b) {}

	Iter begin() const
	{
		return iter_begin;
	}
	Iter end() const
	{
		return iter_end;
	}
};

// Construct a range from 2 iterators
template<typename Iter>
range<Iter> make_range(Iter begin, Iter end)
{
	return {begin, end};
}

// A range of integers
template<typename T>
class int_range {
	T value_begin, value_end;

	static_assert(std::is_integral<T>::value, "int_range can only be used with integral types");

public:
	class iterator {
		T current;

		explicit iterator(T a)
			: current(a) {}
		friend class int_range<T>;

	public:
		typedef T value_type;
		typedef std::ptrdiff_t difference_type;
		typedef iterator pointer;
		typedef T reference;
		typedef std::random_access_iterator_tag iterator_category;

		iterator() = default;

		T operator*() const
		{
			return current;
		}
		T operator[](difference_type offset) const
		{
			return current + offset;
		}

		iterator& operator++()
		{
			++current;
			return *this;
		}
		iterator operator++(int)
		{
			return iterator(current++);
		}
		iterator& operator--()
		{
			--current;
			return *this;
		}
		iterator operator--(int)
		{
			return iterator(current--);
		}

		iterator& operator+=(difference_type offset)
		{
			current += offset;
			return *this;
		}
		iterator& operator-=(difference_type offset)
		{
			current -= offset;
			return *this;
		}

		iterator operator+(difference_type offset) const
		{
			return iterator(current + offset);
		}
		iterator operator-(difference_type offset) const
		{
			return iterator(current - offset);
		}

		friend iterator operator+(difference_type offset, iterator other)
		{
			return other + offset;
		}

		friend difference_type operator-(iterator a, iterator b)
		{
			return a.current - b.current;
		}

		friend bool operator==(iterator a, iterator b)
		{
			return a.current == b.current;
		}
		friend bool operator!=(iterator a, iterator b)
		{
			return a.current != b.current;
		}
		friend bool operator>(iterator a, iterator b)
		{
			return a.current > b.current;
		}
		friend bool operator<(iterator a, iterator b)
		{
			return a.current < b.current;
		}
		friend bool operator>=(iterator a, iterator b)
		{
			return a.current >= b.current;
		}
		friend bool operator<=(iterator a, iterator b)
		{
			return a.current <= b.current;
		}
	};

	int_range(T begin, T end)
		: value_begin(begin), value_end(end) {}

	iterator begin() const
	{
		return iterator(value_begin);
	}
	iterator end() const
	{
		return iterator(value_end);
	}
};

// Construct an int_range between 2 values
template<typename T, typename U>
int_range<typename std::common_type<T, U>::type> irange(T begin, U end)
{
	return {begin, end};
}

} // namespace async
