#ifndef CATTA_NET_EVENTLOOPTHREADPOOL_H
#define CATTA_NET_EVENTLOOPTHREADPOOL_H

#include <catta/base/noncopyable.h>

#include <vector>
#include <memory>

namespace catta {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
	EventLoopThreadPool(EventLoop* baseLoop = nullptr);
	~EventLoopThreadPool();

	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start();

	EventLoop* getLoop();

private:
	EventLoop* baseLoop_;
	bool started_;
	int numThreads_;
	int next_;
	std::vector<std::unique_ptr<EventLoopThread>> threads_;
	std::vector<EventLoop*> loops_;
}; // end class EventLoopThreadPool

} // end namespace catta

#endif // CATTA_NET_EVENTLOOPTHREADPOOL_H