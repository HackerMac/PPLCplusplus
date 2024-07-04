#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Recursively split the arguments so tasks are spawned in parallel
template<std::size_t Start, std::size_t Count>
struct parallel_invoke_internal {
	template<typename Sched, typename Tuple>
	static void run(Sched& sched, const Tuple& args)
	{
		auto&& t = async::local_spawn(sched, [&sched, &args] {
			parallel_invoke_internal<Start + Count / 2, Count - Count / 2>::run(sched, args);
		});
		parallel_invoke_internal<Start, Count / 2>::run(sched, args);
		t.get();
	}
};
template<std::size_t Index>
struct parallel_invoke_internal<Index, 1> {
	template<typename Sched, typename Tuple>
	static void run(Sched&, const Tuple& args)
	{
		// Make sure to preserve the rvalue/lvalue-ness of the original parameter
		std::forward<typename std::tuple_element<Index, Tuple>::type>(std::get<Index>(args))();
	}
};
template<std::size_t Index>
struct parallel_invoke_internal<Index, 0> {
	template<typename Sched, typename Tuple>
	static void run(Sched&, const Tuple&) {}
};

} // namespace detail

// Run several functions in parallel, optionally using the specified scheduler.
template<typename Sched, typename... Args>
typename std::enable_if<detail::is_scheduler<Sched>::value>::type parallel_invoke(Sched& sched, Args&&... args)
{
	detail::parallel_invoke_internal<0, sizeof...(Args)>::run(sched, std::forward_as_tuple(std::forward<Args>(args)...));
}
template<typename... Args>
void parallel_invoke(Args&&... args)
{
	async::parallel_invoke(::async::default_scheduler(), std::forward<Args>(args)...);
}

} // namespace async
