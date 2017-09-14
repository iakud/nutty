#ifndef NUTTY_BASE_POLLPOLLER_H
#define NUTTY_BASE_POLLPOLLER_H

#include <nutty/base/Poller.h>

#include <vector>
#include <map>

struct pollfd;

namespace nutty {

class PollPoller : public Poller {
public:
	PollPoller();
	virtual ~PollPoller();

	virtual void poll(std::vector<Watcher*>& activeWatchers, int timeout);
	virtual void addWatcher(Watcher* watcher);
	virtual void updateWatcher(Watcher* watcher);
	virtual void removeWatcher(Watcher* watcher);

private:
	PollPoller(const PollPoller&) = delete;
	PollPoller& operator=(const PollPoller&) = delete;

	void fillActiveWatchers(int numEvents, std::vector<Watcher*>& activeWatchers);

	int epollfd_;
	std::vector<struct pollfd> fds_;
	std::map<int, size_t> pollfds_;
	std::map<int, Watcher*> watchers_;
}; // end class PollPoller

} // end namespace nutty

#endif // NUTTY_BASE_POLLPOLLER_H