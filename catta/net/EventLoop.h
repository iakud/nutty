#ifndef CATTA_NET_EVENTLOOP_H
#define CATTA_NET_EVENTLOOP_H

#include <catta/util/noncopyable.h>

#include <mutex>
#include <vector>
#include <memory>
#include <functional>

namespace catta {

class Watcher;
class EPollPoller;

class EventLoop : noncopyable {
public:
	typedef std::function<void()> Functor;

public:
	EventLoop();
	virtual ~EventLoop();

	void quit();
	void loop();
	void loopOnce();

	void runInLoop(Functor&& functor);

private:
	void addWatcher(Watcher* watcher);
	void updateWatcher(Watcher* watcher);
	void removeWatcher(Watcher* watcher);

	void handleActiveWatchers();
	void doFunctors();

	void wakeup();
	bool handleWakeup();

	bool quit_;
	std::mutex mutex_;
	std::vector<Functor> functors_;
	std::vector<Watcher*> activeWatchers_;
	std::vector<Watcher*> readyList_;
	std::unique_ptr<EPollPoller> poller_;
	int wakeupFd_;
	std::unique_ptr<Watcher> wakeupWatcher_;

	friend class Watcher;
}; // end class EventLoop

} // end namespace catta

#endif // CATTA_NET_EVENTLOOP_H
