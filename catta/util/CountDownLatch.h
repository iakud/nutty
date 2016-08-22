#ifndef CATTA_UTIL_COUNTDOWNLATCH_H
#define CATTA_UTIL_COUNTDOWNLATCH_H

#include <condition_variable>
#include <mutex>

namespace catta {

class CountDownLatch {
public:
	explicit CountDownLatch(int count);

	void wait();
	void countDown();
	int count();

public:
	// noncopyable
	CountDownLatch(const CountDownLatch&) = delete;
	CountDownLatch& operator=(const CountDownLatch&) = delete;

private:
	std::mutex mutex_;
	std::condition_variable cv_;
	int count_;
}; // end class CountDownLatch

} // end namespace catta

#endif // CATTA_UTIL_COUNTDOWNLATCH_H