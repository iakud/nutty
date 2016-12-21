#include <nutty/base/EventLoopThread.h>

#include <nutty/base/EventLoop.h>

using namespace nutty;

EventLoopThread::EventLoopThread()
	: loop_(nullptr)
	, thread_(&EventLoopThread::threadFunc, this) {
	std::unique_lock<std::mutex> lock(mutex_);
	while (!loop_) {
		cv_.wait(lock);
	}
}

EventLoopThread::~EventLoopThread() {
	loop_->quit();
	thread_.join();
}

void EventLoopThread::threadFunc() {
	EventLoop loop;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		cv_.notify_one();
		loop_ = &loop;
	}
	loop.loop();
	loop_ = nullptr;
}