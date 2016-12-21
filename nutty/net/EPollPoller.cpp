#include <catta/net/EPollPoller.h>

#include <catta/net/Watcher.h>

#include <sys/epoll.h>
#include <unistd.h>

using namespace catta;

EPollPoller::EPollPoller()
	: epollfd_(::epoll_create1(EPOLL_CLOEXEC))
	, events_(kEventSizeInit) {
}

EPollPoller::~EPollPoller() {
	::close(epollfd_);
}

void EPollPoller::poll(std::vector<Watcher*>& activeWatchers, int timeout) {
	int nfd = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeout);
	if (nfd > 0) {
		fillActiveWatchers(nfd, activeWatchers);
		if (nfd == static_cast<int>(events_.size())) {
			events_.resize(events_.size() * 2); // events extend
		}
	} else if (nfd == 0) {

	} else if (errno != EINTR) {
		// error happens
	}
}

void EPollPoller::fillActiveWatchers(int numEvents, std::vector<Watcher*>& activeWatchers) {
	for (int i = 0; i < numEvents; ++i) {
		struct epoll_event& event = events_[i];
		Watcher* watcher = static_cast<Watcher*>(event.data.ptr);
		watcher->containEvents(event.events); // contain triggered events
		activeWatchers.push_back(watcher);
	}
}

void EPollPoller::addWatcher(Watcher* watcher) {
	struct epoll_event event;
	event.events = watcher->events();
	event.data.ptr = watcher;
	if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, watcher->fd(), &event) < 0) {
		// FIXME : on error
	}
}

void EPollPoller::updateWatcher(Watcher* watcher) {
	struct epoll_event event;
	event.events = watcher->events();
	event.data.ptr = watcher;
	if (::epoll_ctl(epollfd_, EPOLL_CTL_MOD, watcher->fd(), &event) < 0) {
		// FIXME : on error
	}
}

void EPollPoller::removeWatcher(Watcher* watcher) {
	if (::epoll_ctl(epollfd_, EPOLL_CTL_DEL, watcher->fd(), nullptr) < 0) {
		// FIXME : on error
	}
}