#ifndef NUTTY_BASE_TIMERQUEUE_H
#define NUTTY_BASE_TIMERQUEUE_H

#include <nutty/base/Watcher.h>
#include <nutty/base/Timer.h>

#include <map>
#include <chrono>
#include <memory>

namespace nutty {

class EventLoop;

typedef std::function<void()> TimerCallback;

class TimerQueue {
public:
	TimerQueue(EventLoop* loop);
	~TimerQueue();

	void addTimer(TimerCallback&& cb,
		const std::chrono::steady_clock::time_point& time,
		const std::chrono::steady_clock::duration& interval = std::chrono::steady_clock::duration::zero());

private:
	typedef std::shared_ptr<Timer> TimerPtr;

	void addTimerInLoop(TimerPtr& timer);
	void handleRead();
	void resetTimer(const std::chrono::steady_clock::time_point& time);

	EventLoop* loop_;
	const int timerfd_;
	Watcher watcher_;

	std::multimap<std::chrono::steady_clock::time_point, TimerPtr> timers_;
}; // end class TimerQueue

} // end namespace nutty

#endif // NUTTY_BASE_TIMERQUEUE_H