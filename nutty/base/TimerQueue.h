#ifndef NUTTY_BASE_TIMERQUEUE_H
#define NUTTY_BASE_TIMERQUEUE_H

#include <nutty/base/Watcher.h>
#include <nutty/base/Callbacks.h>

#include <set>
#include <map>
#include <chrono>
#include <memory>

namespace nutty {

class EventLoop;

class TimerQueue {
public:
	TimerQueue(EventLoop* loop);
	~TimerQueue();

	TimerPtr addTimer(TimerCallback&& cb,
		const std::chrono::steady_clock::time_point& time,
		const std::chrono::steady_clock::duration& interval = std::chrono::steady_clock::duration::zero());
	void cancel(TimerPtr& timer);

private:
	typedef std::multimap<std::chrono::steady_clock::time_point, TimerPtr> TimerMap;

	void addTimerInLoop(TimerPtr& timer);
	void cancelInLoop(TimerPtr& timer);
	void handleRead();
	void resetTimer(const std::chrono::steady_clock::time_point& time);

	EventLoop* loop_;
	const int timerfd_;
	Watcher watcher_;
	int callingExpiredTimers_;
	TimerMap timers_;
	std::set<TimerPtr> cancelingTimers_;
}; // end class TimerQueue

} // end namespace nutty

#endif // NUTTY_BASE_TIMERQUEUE_H