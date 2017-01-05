#ifndef NUTTY_BASE_EVENTLOOP_H
#define NUTTY_BASE_EVENTLOOP_H

#include <nutty/base/Thread.h>

#include <mutex>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <nutty/base/Timer.h>

namespace nutty {

class Watcher;
class EPollPoller;
class TimerQueue;



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

	void runAt(const std::chrono::steady_clock::time_point& time, TimerCallback&& cb);
	void runAfter(const std::chrono::steady_clock::duration& delay, TimerCallback&& cb);
	void runEvery(const std::chrono::steady_clock::duration& interval, TimerCallback&& cb);

	//void cancel(TimerId timerId);

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
	std::unique_ptr<TimerQueue> timerQueue_;
	int wakeupFd_;
	std::unique_ptr<Watcher> wakeupWatcher_;

	std::vector<Watcher*> activeWatchers_;

	std::mutex mutex_;
	std::vector<Functor> pendingFunctors_;

	friend class Watcher;
}; // end class EventLoop

} // end namespace nutty

#endif // NUTTY_BASE_EVENTLOOP_H