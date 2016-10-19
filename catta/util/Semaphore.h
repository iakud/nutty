#ifndef CATTA_UTIL_SEMPHORE_H
#define CATTA_UTIL_SEMPHORE_H

#include <catta/base/noncopyable.h>

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace catta {

class Semaphore : noncopyable {
public:
	explicit Semaphore(int count = 0) : count_(count) {}

	inline void notify() {
		std::unique_lock<std::mutex> lock(mutex_);
		++count_;
		cv_.notify_one();
	}

	inline void wait() {
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ == 0) {
			cv_.wait(lock);
		}
		--count_;
	}

	inline bool try_wait() {
		std::unique_lock<std::mutex> lock(mutex_);
		if (count_ == 0) {
			return false;
		}
		--count_;
		return true;
	}

	template<class Rep, class Period>
	inline bool wait_for(const std::chrono::duration<Rep, Period>& rel_time)  {
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ == 0) {
			if (cv_.wait_for(lock, rel_time) ==  std::cv_status::timeout) {
				return false;
			}
		}
		--count_;
		return true;
	}

	template<class Clock, class Duration>
	inline bool wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) {
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ == 0) {
			if (cv_.wait_until(lock, abs_time) ==  std::cv_status::timeout) {
				return false;
			}
		}
		--count_;
		return true;
	}

private:
	std::mutex mutex_;
	std::condition_variable cv_;
	int count_;
}; // end class Semaphore

} // end namespace catta

#endif // CATTA_UTIL_SEMPHORE_H