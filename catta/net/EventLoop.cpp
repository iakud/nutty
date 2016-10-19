#include <catta/net/EventLoop.h>

#include <catta/net/EPollPoller.h>
#include <catta/net/Watcher.h>

#include <sys/eventfd.h>
#include <unistd.h>

using namespace catta;

namespace {

const int kPollTime = 10000; // ms

int createEventFd() {
	int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (eventFd < 0) {
		abort();
	}
	return eventFd;
}

} // anonymous

EventLoop::EventLoop()
	: quit_(false)
	, callingPendingFunctors_(false)
	, threadId_(CurrentThread::tid())
	, poller_(new EPollPoller())
	, wakeupFd_(createEventFd())
	, wakeupWatcher_(new Watcher(this, wakeupFd_))
	, activeWatchers_()
	, readyList_()
	, mutex_()
	, pendingFunctors_() {
	wakeupWatcher_->setReadCallback(std::bind(&EventLoop::handleWakeup, this));
	wakeupWatcher_->setEvents(WatcherEvents::kEventRead);
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
		poller_->poll(activeWatchers_, kPollTime); // poll network event
		handleActiveWatchers();
		doPendingFunctors();
	}
}

void EventLoop::runInLoop(Functor&& callback) {
	if (isInLoopThread()) {
		callback();
	} else {
		queueInLoop(std::move(callback));
	}
}

void EventLoop::queueInLoop(Functor&& callback) {
	{
		std::unique_lock<std::mutex> lock(mutex_);
		pendingFunctors_.push_back(std::move(callback));
	}
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

void EventLoop::addWatcher(Watcher* watcher) {
	poller_->addWatcher(watcher);
}

void EventLoop::updateWatcher(Watcher* watcher) {
	poller_->updateWatcher(watcher);
}

void EventLoop::removeWatcher(Watcher* watcher) {
	if (watcher->activeIndex() != Watcher::kInvalidActiveIndex) {
		activeWatchers_[watcher->activeIndex()] = nullptr;
		watcher->setActiveIndex(Watcher::kInvalidActiveIndex);
	}
	poller_->removeWatcher(watcher);
}

void EventLoop::handleActiveWatchers() {
	int activeIndex = 0;
	for (Watcher*& watcher : activeWatchers_) {
		if (watcher) {
			watcher->handleEvents();
			if (watcher->revents() != WatcherEvents::kEventNone) {
				activeWatchers_[activeIndex] = watcher;
				watcher->setActiveIndex(activeIndex++);
			} else {
				watcher->setActiveIndex(Watcher::kInvalidActiveIndex);
			}
		}
	}
	if (activeIndex > 0) {
		activeWatchers_.resize(activeIndex);
		wakeup();
	} else {
		activeWatchers_.clear();
	}
}

void EventLoop::doPendingFunctors() {
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for (Functor& callback : functors) {
		callback();
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