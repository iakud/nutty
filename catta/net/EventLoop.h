#ifndef CATTA_NET_EVENTLOOP_H
#define CATTA_NET_EVENTLOOP_H

#include <mutex>
#include <vector>
#include <memory>
#include <functional>

namespace catta {

class Watcher;
class EPollPoller;

class EventLoop {
public:
	typedef std::function<void()> Functor;

public:
	EventLoop();
	virtual ~EventLoop();

	void quit();
	void loop();
	void loopOnce();

	void runInLoop(Functor&& functor);

public:
	// noncopyable
	EventLoop(const EventLoop&) = delete;
	EventLoop& operator=(const EventLoop&) = delete;

private:
	void addWatcher(Watcher* watcher);
	void updateWatcher(Watcher* watcher);
	void removeWatcher(Watcher* watcher);

	void doFunctors();

	void wakeup();
	void handleWakeup();

	bool quit_;
	std::mutex mutex_;
	std::vector<Functor> functors_;
	std::vector<Watcher*> readyList_;
	std::unique_ptr<EPollPoller> poller_;
	int wakeupFd_;
	std::unique_ptr<Watcher> wakeupWatcher_;
}; // end class EventLoop

} // end namespace catta

#endif // IAK_NET_EVENTLOOP_H
