#ifndef NUTTY_BASE_EPOLLPOLLER_H
#define NUTTY_BASE_EPOLLPOLLER_H

#include <vector>

struct epoll_event;

namespace nutty {

class Watcher;

class EPollPoller {
public:
	EPollPoller();
	virtual ~EPollPoller();

	void poll(std::vector<Watcher*>& activeWatchers, int timeout);
	void addWatcher(Watcher* watcher);
	void updateWatcher(Watcher* watcher);
	void removeWatcher(Watcher* watcher);

private:
	static const int kEventSizeInit = 16;

	EPollPoller(const EPollPoller&) = delete;
	EPollPoller& operator=(const EPollPoller&) = delete;

	void fillActiveWatchers(int numEvents, std::vector<Watcher*>& activeWatchers);

	int epollfd_;
	std::vector<struct epoll_event> events_;
}; // end class EPollPoller

} // end namespace nutty

#endif // NUTTY_BASE_EPOLLPOLLER_H