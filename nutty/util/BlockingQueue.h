#ifndef NUTTY_UTIL_BLOCKINGQUEUE_H
#define NUTTY_UTIL_BLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>
#include <assert.h>

namespace nutty {

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

private:
	BlockingQueue(const BlockingQueue&) = delete;
	BlockingQueue& operator=(const BlockingQueue&) = delete;
	
	std::mutex mutex_;
	std::condition_variable cv_;
	std::deque<T> queue_;
}; // end class BlockingQueue

} // end namespace nutty

#endif  // NUTTY_UTIL_BLOCKINGQUEUE_H
