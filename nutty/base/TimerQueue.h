#ifndef NUTTY_BASE_TIMERQUEUE_H
#define NUTTY_BASE_TIMERQUEUE_H

#include <nutty/base/Watcher.h>
#include <nutty/base/Timer.h>

#include <chrono>

namespace nutty {

class EventLoop;

typedef std::function<void()> TimerCallback;

class TimerQueue {
public:
	TimerQueue(EventLoop* loop);
	~TimerQueue();

	void addTimer(TimerCallback&& cb,
		const std::chrono::steady_clock::time_point& when,
		const std::chrono::steady_clock::duration& interval);

private:
	void addTimerInLoop(Timer* timer);
	void handleRead();
	void resetTimer(const std::chrono::steady_clock::time_point& expiration);

	EventLoop* loop_;
	const int timerfd_;
	Watcher watcher_;

	//std::set<std::shared_ptr<Timer>> timers_;
}; // end class TimerQueue

} // end namespace nutty

#endif // NUTTY_BASE_TIMERQUEUE_H