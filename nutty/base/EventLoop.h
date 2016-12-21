#ifndef NUTTY_BASE_EVENTLOOP_H
#define NUTTY_BASE_EVENTLOOP_H

#include <nutty/base/Thread.h>

#include <mutex>
#include <vector>
#include <memory>
#include <functional>

namespace nutty {

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

	void runInLoop(Functor&& cb);
	void queueInLoop(Functor&& cb);

	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
	EventLoop(const EventLoop&) = delete;
	EventLoop& operator=(const EventLoop&) = delete;

	void addWatcher(Watcher* watcher);
	void updateWatcher(Watcher* watcher);
	void removeWatcher(Watcher* watcher);

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

	std::mutex mutex_;
	std::vector<Functor> pendingFunctors_;

	friend class Watcher;
}; // end class EventLoop

} // end namespace nutty

#endif // NUTTY_BASE_EVENTLOOP_H