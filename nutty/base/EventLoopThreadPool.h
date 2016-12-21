#ifndef NUTTY_BASE_EVENTLOOPTHREADPOOL_H
#define NUTTY_BASE_EVENTLOOPTHREADPOOL_H

#include <vector>
#include <memory>

namespace nutty {

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
	EventLoopThreadPool(const EventLoopThreadPool&) = delete;
	EventLoopThreadPool& operator=(const EventLoopThreadPool&) = delete;

	EventLoop* baseLoop_;
	bool started_;
	int numThreads_;
	int next_;
	std::vector<std::unique_ptr<EventLoopThread>> threads_;
	std::vector<EventLoop*> loops_;
}; // end class EventLoopThreadPool

} // end namespace nutty

#endif // NUTTY_BASE_EVENTLOOPTHREADPOOL_H