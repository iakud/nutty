#ifndef NUTTY_BASE_EVENTLOOPTHREAD_H
#define NUTTY_BASE_EVENTLOOPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace nutty {

class EventLoop;

class EventLoopThread {
public:
	EventLoopThread();
	~EventLoopThread();

	EventLoop* getLoop() { return loop_; }

private:
	EventLoopThread(const EventLoopThread&) = delete;
	EventLoopThread& operator=(const EventLoopThread&) = delete;
	
	void threadFunc();

	EventLoop* loop_;
	std::thread thread_;
	std::mutex mutex_;
	std::condition_variable cv_;
}; // end class EventLoopThread

} // end namespace nutty

#endif // NUTTY_BASE_EVENTLOOPTHREAD_H