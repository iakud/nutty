#ifndef NUTTY_BASE_POLLER_H
#define NUTTY_BASE_POLLER_H

#include <vector>

namespace nutty {

class Watcher;

class Poller {
public:
	static Poller* createDefault();

	Poller();
	virtual ~Poller();

	virtual void poll(std::vector<Watcher*>& activeWatchers, int timeout) = 0;
	virtual void addWatcher(Watcher* watcher) = 0;
	virtual void updateWatcher(Watcher* watcher) = 0;
	virtual void removeWatcher(Watcher* watcher) = 0;

private:

}; // end class Poller

} // end namespace nutty

#endif // NUTTY_BASE_POLLER_H