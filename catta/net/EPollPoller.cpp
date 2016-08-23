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
	::free(events_);
	::close(epollFd_);
}

void EPollPoller::poll(std::vector<Watcher*>& readyList, int timeout) {
	int nfd = ::epoll_wait(epollFd_, events_.data(), events.size(), timeout);
	if (nfd > 0) {
		for (int i = 0; i < nfd; ++i) {
			struct epoll_event& event = events_[i];
			Watcher* watcher = static_cast<Watcher*>(event.data.ptr);
			if (event.events & (EPOLLRDHUP|EPOLLHUP)) {
				watcher->onClose();
			}
			if (event.events & (EPOLLERR)) {
				watcher->onError();
			}
			if (event.events & (EPOLLIN)) {
				watcher->onRead();
			}
			if (event.events & (EPOLLOUT)) {
				watcher->onWrite();
			}
		}
		if (nfd == events_.size()) {
			events_.resize(events_.size() * 2); // events extend
		}
	} else if (nfd == 0) {

	} else if (errno != EINTR) {
		// error happens
	}
}

void EPollPoller::addWatcher(Watcher* watcher) {
	struct epoll_event event;
	if (watcher->isRead()) {
		event.events |= EPOLLIN;
	}
	if (watcher->isWrite()) {
		event.events |= EPOLLOUT;
	}
	event.events |= (EPOLLERR|EPOLLRDHUP|EPOLLET);	// edge trigger
	event.data.ptr = watcher;
	if (::epoll_ctl(epollFd_, EPOLL_CTL_ADD, watcher->getFd(), &event) < 0) {
		// FIXME : on error
	}
}

void EPollPoller::updateWatcher(Watcher* watcher) {
	struct epoll_event event;
	if (channel->isRead()) {
		event.events |= EPOLLIN;
	}
	if (channel->isWrite()) {
		event.events |= EPOLLOUT;
	}
	event.events |= (EPOLLERR|EPOLLRDHUP|EPOLLET);	// edge trigger
	event.data.ptr = channel;
	if (::epoll_ctl(epollFd_, EPOLL_CTL_MOD, watcher->getFd(), &event) < 0) {
		// FIXME : on error
	}
}

void EPollPoller::removeWatcher(Watcher* watcher) {
	if (::epoll_ctl(epollFd_, EPOLL_CTL_DEL, watcher->getFd(), nullptr) < 0) {
		// FIXME : on error
	}
}