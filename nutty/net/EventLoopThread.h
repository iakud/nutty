#ifndef CATTA_NET_EVENTLOOPTHREAD_H
#define CATTA_NET_EVENTLOOPTHREAD_H

#include <catta/base/noncopyable.h>

#include <thread>
#include <mutex>
#include <condition_variable>

namespace catta {

class EventLoop;

class EventLoopThread : noncopyable {
public:
	EventLoopThread();
	~EventLoopThread();

	EventLoop* getLoop() { return loop_; }

private:
	void threadFunc();

	EventLoop* loop_;
	std::thread thread_;
	std::mutex mutex_;
	std::condition_variable cv_;
}; // end class EventLoopThread

} // end namespace catta

#endif // CATTA_NET_EVENTLOOPTHREAD_H