#ifndef ASYNCXX_H_
# error "Do not include this header directly, include <async++.h> instead."
#endif

namespace async {
namespace detail {

// Queue which holds tasks in FIFO order. Note that this queue is not
// thread-safe and must be protected by a lock.
class fifo_queue {
	detail::aligned_array<void*, LIBASYNC_CACHELINE_SIZE> items;
	std::size_t head, tail;

public:
	fifo_queue()
		: items(32), head(0), tail(0) {}
	~fifo_queue()
	{
		// Free any unexecuted tasks
		for (std::size_t i = head; i != tail; i = (i + 1) & (items.size() - 1))
			task_run_handle::from_void_ptr(items[i]);
	}

	// Push a task to the end of the queue
	void push(task_run_handle t)
	{
		// Resize queue if it is full
		if (head == ((tail + 1) & (items.size() - 1))) {
			detail::aligned_array<void*, LIBASYNC_CACHELINE_SIZE> new_items(items.size() * 2);
			for (std::size_t i = 0; i != items.size(); i++)
				new_items[i] = items[(i + head) & (items.size() - 1)];
			head = 0;
			tail = items.size() - 1;
			items = std::move(new_items);
		}

		// Push the item
		items[tail] = t.to_void_ptr();
		tail = (tail + 1) & (items.size() - 1);
	}

	// Pop a task from the front of the queue
	task_run_handle pop()
	{
		// See if an item is available
		if (head == tail)
			return task_run_handle();
		else {
			void* x = items[head];
			head = (head + 1) & (items.size() - 1);
			return task_run_handle::from_void_ptr(x);
		}
	}
};

} // namespace detail
} // namespace async
