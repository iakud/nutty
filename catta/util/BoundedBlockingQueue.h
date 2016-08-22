#ifndef CATTA_UTIL_BOUNDEDBLOCKINGQUEUE_H
#define CATTA_UTIL_BOUNDEDBLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>
#include <assert.h>

namespace catta {

template<typename T>
class BoundedBlockingQueue {

public:
	explicit BoundedBlockingQueue(size_t capacity)
		: mutex_()
		, cvne_()
		, cvnf_()
		, capacity_(capacity)
		, queue_() {
	}

	void put(const T& value) {
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.full()) {
			cvnf_.wait(lock);
		}
		assert(!queue_.full());
		queue_.push_back(value);
		cvne_.notify_one();
	}

	T take() {
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()) {
			cvne_.wait();
		}
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		cvnf_.notify_one();
		return front;
	}

	bool empty() {
		std::unique_lock<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	bool full() {
		std::unique_lock<std::mutex> lock(mutex_);
		return queue_.size() >= capacity_;
	}

	size_t size() {
		std::unique_lock<std::mutex> lock(mutex_);
		return queue_.size();
	}

	size_t capacity() const {
		return capacity_;
	}

public:
	// noncopyable
	BoundedBlockingQueue(const BoundedBlockingQueue&) = delete;
	BoundedBlockingQueue& operator=(const BoundedBlockingQueue&) = delete;

private:
	std::mutex mutex_;
	std::condition_variable cvne_; // not empty
	std::condition_variable cvnf_; // not full
	size_t capacity_;
	std::deque<T> queue_;
}; // end class BoundedBlockingQueue

} // end namespace catta

#endif  // CATTA_UTIL_BOUNDEDBLOCKINGQUEUE_H