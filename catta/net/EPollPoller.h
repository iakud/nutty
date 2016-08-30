#ifndef CATTA_NET_EPOLLPOLLER_H
#define CATTA_NET_EPOLLPOLLER_H

#include <vector>

struct epoll_event;

namespace catta {

class Watcher;

class EPollPoller {
protected:
	static const int kEventSizeInit = 16;
public:
	EPollPoller();
	virtual ~EPollPoller();

	void poll(std::vector<Watcher*>& readyList, int timeout);
	void addWatcher(Watcher* watcher);
	void updateWatcher(Watcher* watcher);
	void removeWatcher(Watcher* watcher);

public:
	// noncopyable
	EPollPoller(const EPollPoller&) = delete;
	EPollPoller& operator=(const EPollPoller&) = delete;

private:
	int epollFd_;
	std::vector<struct epoll_event> events_;
}; // end class EPollPoller

} // end namespace catta

#endif // CATTA_NET_EPOLLPOLLER_H