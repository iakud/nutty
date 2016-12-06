#include <catta/net/EventLoopThread.h>

#include <catta/net/EventLoop.h>

using namespace catta;

EventLoopThread::EventLoopThread()
	: loop_(nullptr)
	, thread_(std::bind(&EventLoopThread::threadFunc, this)) {
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