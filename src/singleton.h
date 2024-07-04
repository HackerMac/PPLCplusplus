namespace async {
namespace detail {

// Thread-safe singleton wrapper class
#ifdef HAVE_THREAD_SAFE_STATIC
// C++11 guarantees thread safety for static initialization
template<typename T>
class singleton {
public:
	static T& get_instance()
	{
		static T instance;
		return instance;
	}
};
#else
// Some compilers don't support thread-safe static initialization, so emulate it
template<typename T>
class singleton {
	std::mutex lock;
	std::atomic<bool> init_flag;
	alignas(T) std::uint8_t storage[sizeof(T)];

	static singleton instance;

	// Use a destructor instead of atexit() because the latter does not work
	// properly when the singleton is in a library that is unloaded.
	~singleton()
	{
		if (init_flag.load(std::memory_order_acquire))
			reinterpret_cast<T*>(&storage)->~T();
	}

public:
	static T& get_instance()
	{
		T* ptr = reinterpret_cast<T*>(&instance.storage);
		if (!instance.init_flag.load(std::memory_order_acquire)) {
			std::lock_guard<std::mutex> locked(instance.lock);
			if (!instance.init_flag.load(std::memory_order_relaxed)) {
				new(ptr) T;
				instance.init_flag.store(true, std::memory_order_release);
			}
		}
		return *ptr;
	}
};

template<typename T> singleton<T> singleton<T>::instance;
#endif

} // namespace detail
} // namespace async
