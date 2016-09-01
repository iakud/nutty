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
	, mutex_()
	, functors_()
	, readyList_()
	, poller_(new EPollPoller())
	, wakeupFd_(createEventFd())
	, wakeupWatcher_(new Watcher(wakeupFd_, this)) {
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
	wakeup();
}

void EventLoop::loop() {
	quit_ = false;
	// blocking until quit
	while(!quit_) {
		poller_->poll(activeWatchers_, kPollTime); // poll network event

		std::vector<Watcher*> activeWatchers;
		activeWatchers.swap(activeWatchers_);
		for (Watcher*& watcher : activeWatchers) {
			if (watcher) {
				watcher->handleEvents();
			}
		}
		if (!activeWatchers_.empty()) {
			wakeup();
		}

		doFunctors();
	}
}

void EventLoop::runInLoop(Functor&& functor) {
	{
		std::unique_lock<std::mutex> lock(mutex_);
		functors_.push_back(functor);
	}
	wakeup();
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

void EventLoop::doFunctors() {
	std::vector<Functor> functors;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		functors.swap(functors_);
	}
	for (Functor functor : functors) {
		functor();
	}
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