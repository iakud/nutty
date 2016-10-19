#ifndef CATTA_NET_EVENTLOOP_H
#define CATTA_NET_EVENTLOOP_H

#include <catta/base/noncopyable.h>
#include <catta/base/Thread.h>

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

	void runInLoop(Functor&& callback);
	void queueInLoop(Functor&& callback);

	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
	void addWatcher(Watcher* watcher);
	void updateWatcher(Watcher* watcher);
	void removeWatcher(Watcher* watcher);

	void handleActiveWatchers();
	void doPendingFunctors();

	void wakeup();
	bool handleWakeup();

	bool quit_;
	bool callingPendingFunctors_;
	const pid_t threadId_;
	std::unique_ptr<EPollPoller> poller_;
	int wakeupFd_;
	std::unique_ptr<Watcher> wakeupWatcher_;

	std::vector<Watcher*> activeWatchers_;
	std::vector<Watcher*> readyList_;

	std::mutex mutex_;
	std::vector<Functor> pendingFunctors_;

	friend class Watcher;
}; // end class EventLoop

} // end namespace catta

#endif // CATTA_NET_EVENTLOOP_H
