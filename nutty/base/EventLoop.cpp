#include <nutty/base/EventLoop.h>

#include <nutty/base/EPollPoller.h>
#include <nutty/base/TimerQueue.h>
#include <nutty/base/Watcher.h>

#include <sys/eventfd.h>
#include <unistd.h>

namespace {

const int kPollTime = 10000; // ms

int createEventfd() {
	int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (eventFd < 0) {
		::abort(); // FIXME log
	}
	return eventFd;
}

} // end anonymous namespace

using namespace nutty;

EventLoop::EventLoop()
	: quit_(false)
	, callingPendingFunctors_(false)
	, threadId_(CurrentThread::tid())
	, poller_(new EPollPoller())
	, timerQueue_(new TimerQueue(this))
	, wakeupFd_(createEventfd())
	, wakeupWatcher_(new Watcher(this, wakeupFd_))
	, activeWatchers_()
	, mutex_()
	, pendingFunctors_() {
	wakeupWatcher_->setReadCallback(std::bind(&EventLoop::handleWakeup, this));
	wakeupWatcher_->enableReading();
	wakeupWatcher_->start();
}

EventLoop::~EventLoop() {
	wakeupWatcher_->stop();
	::close(wakeupFd_);
}

void EventLoop::quit() {
	quit_ = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}

void EventLoop::loop() {
	quit_ = false;
	// blocking until quit
	while(!quit_) {
		activeWatchers_.clear();
		poller_->poll(activeWatchers_, kPollTime); // poll network event
		for (Watcher*& watcher : activeWatchers_) {
			watcher->handleEvents();
		}
		doPendingFunctors();
	}
}

void EventLoop::runInLoop(Functor&& cb) {
	if (isInLoopThread()) {
		cb();
	} else {
		queueInLoop(std::move(cb));
	}
}

void EventLoop::queueInLoop(Functor&& cb) {
	{
		std::unique_lock<std::mutex> lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

void EventLoop::runAt(const std::chrono::steady_clock::time_point& time, TimerCallback&& cb) {
	timerQueue_->addTimer(std::move(cb), time);
}

void EventLoop::runAfter(const std::chrono::steady_clock::duration& delay, TimerCallback&& cb) {
	timerQueue_->addTimer(std::move(cb), std::chrono::steady_clock::now() + delay);
}

void EventLoop::runEvery(const std::chrono::steady_clock::duration& interval, TimerCallback&& cb) {
	timerQueue_->addTimer(std::move(cb), std::chrono::steady_clock::now() + interval, interval);
}

void EventLoop::addWatcher(Watcher* watcher) {
	poller_->addWatcher(watcher);
}

void EventLoop::updateWatcher(Watcher* watcher) {
	poller_->updateWatcher(watcher);
}

void EventLoop::removeWatcher(Watcher* watcher) {
	poller_->removeWatcher(watcher);
}

void EventLoop::doPendingFunctors() {
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for (Functor& cb : functors) {
		cb();
	}
	callingPendingFunctors_ = false;
}

void EventLoop::wakeup() {
	uint64_t flag = 1;
	ssize_t n = ::write(wakeupFd_, &flag, sizeof flag);
	if (n != sizeof flag) {
		// FIXME : on error
	}
}

bool EventLoop::handleWakeup() {
	uint64_t flag;
	ssize_t n = ::read(wakeupFd_, &flag, sizeof flag);
	if (n != sizeof flag) {
		// FIXME : on error
	}
	return false;
}