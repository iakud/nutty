#include <catta/net/EventLoopThreadPool.h>

#include <catta/net/EventLoopThread.h>

using namespace catta;

EventLoopThreadPool::EventLoopThreadPool(int numThreads)
	: numThreads_(numThreads)
	, next_(0) {
	for (int i = 0; i < numThreads; ++i) {
		EventLoopThread* thread = new EventLoopThread();
		threads_.push_back(std::unique_ptr<EventLoopThread>(thread));
		loops_.push_back(thread->getLoop());
	}
}

EventLoopThreadPool::~EventLoopThreadPool() {
}

EventLoop* EventLoopThreadPool::getLoop() {
	if (loops_.empty()) {
		return nullptr;
	}

	EventLoop* loop = loops_[next_];
	++next_;
	if (static_cast<size_t>(next_) >= loops_.size()) {
		next_ = 0;
	}
	return loop;
}