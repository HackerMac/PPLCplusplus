#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Default map function which simply passes its parameter through unmodified
struct default_map {
	template<typename T>
	T&& operator()(T&& x) const
	{
		return std::forward<T>(x);
	}
};

// Internal implementation of parallel_map_reduce that only accepts a
// partitioner argument.
template<typename Sched, typename Partitioner, typename Result, typename MapFunc, typename ReduceFunc>
Result internal_parallel_map_reduce(Sched& sched, Partitioner partitioner, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	// Split the partition, run inline if no more splits are possible
	auto subpart = partitioner.split();
	if (subpart.begin() == subpart.end()) {
		Result out = init;
		for (auto&& i: partitioner)
			out = reduce(std::move(out), map(std::forward<decltype(i)>(i)));
		return out;
	}

	// Run the function over each half in parallel
	auto&& t = async::local_spawn(sched, [&sched, &subpart, init, &map, &reduce] {
		return detail::internal_parallel_map_reduce(sched, std::move(subpart), init, map, reduce);
	});
	Result out = detail::internal_parallel_map_reduce(sched, std::move(partitioner), init, map, reduce);
	return reduce(std::move(out), t.get());
}

} // namespace detail

// Run a function for each element in a range and then reduce the results of that function to a single value
template<typename Sched, typename Range, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(Sched& sched, Range&& range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return detail::internal_parallel_map_reduce(sched, async::to_partitioner(std::forward<Range>(range)), init, map, reduce);
}

// Overload with default scheduler
template<typename Range, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(Range&& range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(::async::default_scheduler(), range, init, map, reduce);
}

// Overloads with std::initializer_list
template<typename Sched, typename T, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(Sched& sched, std::initializer_list<T> range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(sched, async::make_range(range.begin(), range.end()), init, map, reduce);
}
template<typename T, typename Result, typename MapFunc, typename ReduceFunc>
Result parallel_map_reduce(std::initializer_list<T> range, Result init, const MapFunc& map, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(async::make_range(range.begin(), range.end()), init, map, reduce);
}

// Variant with identity map operation
template<typename Sched, typename Range, typename Result, typename ReduceFunc>
Result parallel_reduce(Sched& sched, Range&& range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_map_reduce(sched, range, init, detail::default_map(), reduce);
}
template<typename Range, typename Result, typename ReduceFunc>
Result parallel_reduce(Range&& range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_reduce(::async::default_scheduler(), range, init, reduce);
}
template<typename Sched, typename T, typename Result, typename ReduceFunc>
Result parallel_reduce(Sched& sched, std::initializer_list<T> range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_reduce(sched, async::make_range(range.begin(), range.end()), init, reduce);
}
template<typename T, typename Result, typename ReduceFunc>
Result parallel_reduce(std::initializer_list<T> range, Result init, const ReduceFunc& reduce)
{
	return async::parallel_reduce(async::make_range(range.begin(), range.end()), init, reduce);
}

} // namespace async
