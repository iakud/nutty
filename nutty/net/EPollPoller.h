#ifndef CATTA_NET_EPOLLPOLLER_H
#define CATTA_NET_EPOLLPOLLER_H

#include <catta/base/noncopyable.h>

#include <vector>

struct epoll_event;

namespace catta {

class Watcher;

class EPollPoller : noncopyable {
public:
	EPollPoller();
	virtual ~EPollPoller();

	void poll(std::vector<Watcher*>& activeWatchers, int timeout);
	void addWatcher(Watcher* watcher);
	void updateWatcher(Watcher* watcher);
	void removeWatcher(Watcher* watcher);

private:
	static const int kEventSizeInit = 16;

	void fillActiveWatchers(int numEvents, std::vector<Watcher*>& activeWatchers);

	int epollfd_;
	std::vector<struct epoll_event> events_;
}; // end class EPollPoller

} // end namespace catta

#endif // CATTA_NET_EPOLLPOLLER_H