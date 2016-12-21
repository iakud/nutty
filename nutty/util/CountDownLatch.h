#ifndef NUTTY_UTIL_COUNTDOWNLATCH_H
#define NUTTY_UTIL_COUNTDOWNLATCH_H

#include <condition_variable>
#include <mutex>

namespace nutty {

class CountDownLatch {
public:
	explicit CountDownLatch(int count);

	void wait();
	void countDown();
	int count();

private:
	CountDownLatch(const CountDownLatch&) = delete;
	CountDownLatch& operator=(const CountDownLatch&) = delete;

	std::mutex mutex_;
	std::condition_variable cv_;
	int count_;
}; // end class CountDownLatch

} // end namespace nutty

#endif // NUTTY_UTIL_COUNTDOWNLATCH_H