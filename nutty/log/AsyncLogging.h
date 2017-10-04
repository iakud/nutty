#ifndef NUTTY_LOG_ASYNCLOGGING_H
#define NUTTY_LOG_ASYNCLOGGING_H

#include "AsyncLogger.h"

#include <base/CountDownLatch.h>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <vector>

namespace nutty {

class AsyncLogging {

public:
	AsyncLogging();
	~AsyncLogging();

	void append(AsyncLoggerPtr& asyncLogger, const char* logline, int len);

	void start() {
		running_ = true;
		thread_ = std::thread(&AsyncLogging::threadFunc, this);
		latch_.wait();
	}

	void stop() {
		running_ = false;
		cv_.notify_one();
		thread_.join();
	}

private:
	// noncopyable
	AsyncLogging(const AsyncLogging&) = delete;
	AsyncLogging& operator=(const AsyncLogging&) = delete；
	
	void threadFunc();
	void swapAsyncLoggersToWrite(std::vector<AsyncLoggerPtr>& asyncLoggersToWrite);

	volatile bool running_;
	std::thread thread_;
	base::CountDownLatch latch_;
	std::mutex mutex_;
	std::condition_variable cv_;

	std::vector<AsyncLoggerPtr> asyncLoggers_;
}; // end class AsyncLogging

} // end namespace nutty

#endif  // NUTTY_LOG_ASYNCLOGGING_H
