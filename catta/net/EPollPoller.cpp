#include <catta/net/EPollPoller.h>

#include <catta/net/Watcher.h>

#include <sys/epoll.h>
#include <unistd.h>

using namespace catta;

EPollPoller::EPollPoller()
	: epollFd_(::epoll_create1(EPOLL_CLOEXEC))
	, events_(kEventSizeInit) {
}

EPollPoller::~EPollPoller() {
	::close(epollFd_);
}

void EPollPoller::poll(std::vector<Watcher*>& readyList, int timeout) {
	int nfd = ::epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeout);
	if (nfd > 0) {
		for (int i = 0; i < nfd; ++i) {
			struct epoll_event& event = events_[i];
			Watcher* watcher = static_cast<Watcher*>(event.data.ptr);
			bool readied = (watcher->revents() != WatcherEvents::kEventNone);
			WatcherEvents revents = WatcherEvents::kEventNone;
			if (event.events & (EPOLLRDHUP|EPOLLHUP)) {
				revents |= WatcherEvents::kEventClose;
			}
			if (event.events & (EPOLLERR)) {
				revents |= WatcherEvents::kEventError;
			}
			if (event.events & (EPOLLIN)) {
				revents |= WatcherEvents::kEventRead;
			}
			if (event.events & (EPOLLOUT)) {
				revents |= WatcherEvents::kEventWrite;
			}
			watcher->containTriggeredEvents(revents);
			if (!readied) {
				readyList.push_back(watcher);
			}
		}
		if (nfd == static_cast<int>(events_.size())) {
			events_.resize(events_.size() * 2); // events extend
		}
	} else if (nfd == 0) {

	} else if (errno != EINTR) {
		// error happens
	}
}

void EPollPoller::addWatcher(Watcher* watcher) {
	struct epoll_event event;
	event.events = (EPOLLERR|EPOLLRDHUP|EPOLLET);	// edge trigger
	WatcherEvents events = watcher->events();
	if (events & WatcherEvents::kEventRead) {
		event.events |= EPOLLIN;
	}
	if (events & WatcherEvents::kEventWrite) {
		event.events |= EPOLLOUT;
	}
	event.data.ptr = watcher;
	if (::epoll_ctl(epollFd_, EPOLL_CTL_ADD, watcher->fd(), &event) < 0) {
		// FIXME : on error
	}
}

void EPollPoller::updateWatcher(Watcher* watcher) {
	struct epoll_event event;
	event.events = (EPOLLERR|EPOLLRDHUP|EPOLLET);	// edge trigger
	WatcherEvents events = watcher->events();
	if (events & WatcherEvents::kEventRead) {
		event.events |= EPOLLIN;
	}
	if (events & WatcherEvents::kEventWrite) {
		event.events |= EPOLLOUT;
	}
	event.data.ptr = watcher;
	if (::epoll_ctl(epollFd_, EPOLL_CTL_MOD, watcher->fd(), &event) < 0) {
		// FIXME : on error
	}
}

void EPollPoller::removeWatcher(Watcher* watcher) {
	if (::epoll_ctl(epollFd_, EPOLL_CTL_DEL, watcher->fd(), nullptr) < 0) {
		// FIXME : on error
	}
}