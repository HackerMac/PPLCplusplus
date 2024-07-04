#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {

// Exception thrown by cancel_current_task()
struct LIBASYNC_EXPORT_EXCEPTION task_canceled {};

// A flag which can be used to request cancellation
class cancellation_token {
	std::atomic<bool> state;

public:
	cancellation_token()
		: state(false) {}

	// Non-copyable and non-movable
	cancellation_token(const cancellation_token&) = delete;
	cancellation_token& operator=(const cancellation_token&) = delete;

	bool is_canceled() const
	{
		bool s = state.load(std::memory_order_relaxed);
		if (s)
			std::atomic_thread_fence(std::memory_order_acquire);
		return s;
	}

	void cancel()
	{
		state.store(true, std::memory_order_release);
	}

	void reset()
	{
		state.store(false, std::memory_order_relaxed);
	}
};

// Interruption point, throws task_canceled if the specified token is set.
inline void interruption_point(const cancellation_token& token)
{
	if (token.is_canceled())
		LIBASYNC_THROW(task_canceled());
}

} // namespace async
