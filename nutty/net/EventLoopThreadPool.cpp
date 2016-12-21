#include <catta/net/EventLoopThreadPool.h>

#include <catta/net/EventLoopThread.h>

using namespace catta;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
	: baseLoop_(baseLoop)
	, started_(false)
	, numThreads_(0)
	, next_(0) {
	
}

EventLoopThreadPool::~EventLoopThreadPool() {
}

void EventLoopThreadPool::start() {
	if (started_) {
		return;
	}
	started_ = true;
	for (int i = 0; i < numThreads_; ++i) {
		EventLoopThread* thread = new EventLoopThread();
		threads_.push_back(std::unique_ptr<EventLoopThread>(thread));
		loops_.push_back(thread->getLoop());
	}
}

EventLoop* EventLoopThreadPool::getLoop() {
	if (loops_.empty()) {
		return baseLoop_;
	}

	EventLoop* loop = loops_[next_];
	++next_;
	if (static_cast<size_t>(next_) >= loops_.size()) {
		next_ = 0;
	}
	return loop;
}