#ifndef CATTA_UTIL_BLOCKINGQUEUE_H
#define CATTA_UTIL_BLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>
#include <assert.h>

namespace catta {

template<typename T>
class BlockingQueue {
public:
	BlockingQueue()
		: mutex_()
		, cv_()
		, queue_() {
	}

	void put(const T& value) {
		std::unique_lock<std::mutex> lock(mutex_);
		queue_.push_back(value);
		cv_.notify_one();
	}

	T take() {
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()) {
			cv_.wait(lock);
		}
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	size_t size() {
		std::unique_lock<std::mutex> lock(mutex_);
		return queue_.size();
	}

public:
	// noncopyable
	BlockingQueue(const BlockingQueue&) = delete;
	BlockingQueue& operator=(const BlockingQueue&) = delete;

private:
	std::mutex mutex_;
	std::condition_variable cv_;
	std::deque<T> queue_;
}; // end class BlockingQueue

} // end namespace catta

#endif  // CATTA_UTIL_BLOCKINGQUEUE_H
