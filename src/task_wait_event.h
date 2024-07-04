namespace async {
namespace detail {

// Set of events that an task_wait_event can hold
enum wait_type {
	// The task that is being waited on has completed
	task_finished = 1,

	// A task is available to execute from the scheduler
	task_available = 2
};

// OS-supported event object which can be used to wait for either a task to
// finish or for the scheduler to have more work for the current thread.
//
// The event object is lazily initialized to avoid unnecessary API calls.
class task_wait_event {
	alignas(std::mutex) std::uint8_t m[sizeof(std::mutex)];
	alignas(std::condition_variable) std::uint8_t c[sizeof(std::condition_variable)];
	int event_mask;
	bool initialized;

	std::mutex& mutex()
	{
		return *reinterpret_cast<std::mutex*>(&m);
	}
	std::condition_variable& cond()
	{
		return *reinterpret_cast<std::condition_variable*>(&c);
	}

public:
	task_wait_event()
		: event_mask(0), initialized(false) {}

	~task_wait_event()
	{
		if (initialized) {
			mutex().~mutex();
			cond().~condition_variable();
		}
	}

	// Initialize the event, must be done before any other functions are called.
	void init()
	{
		if (!initialized) {
			new(&m) std::mutex;
			new(&c) std::condition_variable;
			initialized = true;
		}
	}

	// Wait for an event to occur. Returns the event(s) that occurred. This also
	// clears any pending events afterwards.
	int wait()
	{
		std::unique_lock<std::mutex> lock(mutex());
		while (event_mask == 0)
			cond().wait(lock);
		int result = event_mask;
		event_mask = 0;
		return result;
	}

	// Check if a specific event is ready
	bool try_wait(int event)
	{
		std::lock_guard<std::mutex> lock(mutex());
		int result = event_mask & event;
		event_mask &= ~event;
		return result != 0;
	}

	// Signal an event and wake up a sleeping thread
	void signal(int event)
	{
		std::unique_lock<std::mutex> lock(mutex());
		event_mask |= event;

		// This must be done while holding the lock otherwise we may end up with
		// a use-after-free due to a race with wait().
		cond().notify_one();
		lock.unlock();
	}
};

} // namespace detail
} // namespace async
