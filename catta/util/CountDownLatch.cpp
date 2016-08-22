#include <catta/util/CountDownLatch.h>

using namespace catta;

CountDownLatch::CountDownLatch(int count)
	: mutex_()
	, cv_()
	, count_(count) {

}

void CountDownLatch::wait() {
	std::unique_lock<std::mutex> lock(mutex_);
	while (count_ > 0) {
		cv_.wait(lock);
	}
}

void CountDownLatch::countDown() {
	std::unique_lock<std::mutex> lock(mutex_);
	--count_;
	if (count_ == 0) {
		cv_.notify_all();
	}
}

int CountDownLatch::count() {
	std::unique_lock<std::mutex> lock(mutex_);
	return count_;
}