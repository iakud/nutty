#include <nutty/base/poller/EPollPoller.h>

#include <nutty/base/Watcher.h>

#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace nutty;

static_assert(EPOLLIN == POLLIN, "EPOLLIN != POLLIN");
static_assert(EPOLLPRI == POLLPRI, "EPOLLPRI != POLLPRI");
static_assert(EPOLLOUT == POLLOUT, "EPOLLOUT != POLLOUT");
static_assert(EPOLLERR == POLLERR, "EPOLLERR != POLLERR");
static_assert(EPOLLHUP == POLLHUP, "EPOLLHUP != POLLHUP");
static_assert(EPOLLRDHUP == POLLRDHUP, "EPOLLRDHUP != POLLRDHUP");

EPollPoller::EPollPoller()
	: Poller()
	, epollfd_(::epoll_create1(EPOLL_CLOEXEC))
	, events_(kEventSizeInit) {
}

EPollPoller::~EPollPoller() {
	::close(epollfd_);
}

void EPollPoller::poll(std::vector<Watcher*>& activeWatchers, int timeout) {
	int numEvents = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeout);
	if (numEvents > 0) {
		fillActiveWatchers(numEvents, activeWatchers);
		if (numEvents == static_cast<int>(events_.size())) {
			events_.resize(events_.size() * 2); // events extend
		}
	} else if (numEvents == 0) {

	} else if (errno != EINTR) {
		// error happens
	}
}

void EPollPoller::fillActiveWatchers(int numEvents, std::vector<Watcher*>& activeWatchers) {
	for (int i = 0; i < numEvents; ++i) {
		struct epoll_event& event = events_[i];
		Watcher* watcher = static_cast<Watcher*>(event.data.ptr);
		watcher->revents(event.events); // contain triggered events
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