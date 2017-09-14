#include <nutty/base/poller/PollPoller.h>

#include <nutty/base/Watcher.h>

#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace nutty;

PollPoller::PollPoller()
	: Poller() 
	, epollfd_(::epoll_create1(EPOLL_CLOEXEC)) {
}

PollPoller::~PollPoller() {
	::close(epollfd_);
}

void PollPoller::poll(std::vector<Watcher*>& activeWatchers, int timeout) {
	int numEvents = ::poll(fds_.data(), fds_.size(), timeout);
	if (numEvents > 0) {
		fillActiveWatchers(numEvents, activeWatchers);
	} else if (numEvents == 0) {

	} else if (errno != EINTR) {
		// error happens
	}
}

void PollPoller::fillActiveWatchers(int numEvents, std::vector<Watcher*>& activeWatchers) {
	for (std::vector<struct pollfd>::iterator it = fds_.begin(); it != fds_.end() && numEvents > 0; ++it) {
		struct pollfd& pfd = *it;
		if (pfd.revents > 0) {
			--numEvents;
			Watcher*& watcher = watchers_[pfd.fd];
			watcher->revents(pfd.revents);
			pfd.revents = 0;
			activeWatchers.push_back(watcher);
		}
	}
}

void PollPoller::addWatcher(Watcher* watcher) {
	pollfds_[watcher->fd()] = fds_.size();
	fds_.emplace_back();
	struct pollfd& pfd = fds_.back();
	pfd.fd = watcher->fd();
	pfd.events = static_cast<short>(watcher->events());
	pfd.revents = 0;
	watchers_[pfd.fd] = watcher;
}

void PollPoller::updateWatcher(Watcher* watcher) {
	std::map<int, size_t>::iterator it = pollfds_.find(watcher->fd());
	if (it != pollfds_.end()) {
		struct pollfd& pfd = fds_[it->second];
		pfd.events = static_cast<short>(watcher->events());
		pfd.revents = 0;
	}
}

void PollPoller::removeWatcher(Watcher* watcher) {
	std::map<int, size_t>::iterator it = pollfds_.find(watcher->fd());
	if (it != pollfds_.end()) {
		struct pollfd& pfd = fds_[it->second];
		watchers_.erase(pfd.fd);
		pollfds_.erase(pfd.fd);
		if (pfd.fd != fds_.back().fd) {
			pfd = fds_.back();
			pollfds_[pfd.fd] = it->second;
		}
		fds_.pop_back();
	}
}