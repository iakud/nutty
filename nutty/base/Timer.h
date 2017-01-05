#ifndef NUTTY_BASE_TIMER_H
#define NUTTY_BASE_TIMER_H

#include <chrono>
#include <functional>

namespace nutty {

typedef std::function<void()> TimerCallback;

class Timer {
public:
	Timer(TimerCallback&& cb,
		const std::chrono::steady_clock::time_point& time,
		const std::chrono::steady_clock::duration& interval)
		: timerCallback_(std::move(cb))
		, expiration_(time)
		, interval_(interval) {

	}

	const std::chrono::steady_clock::time_point& expiration() const { return expiration_; }

	bool repeat() { return interval_ != std::chrono::steady_clock::duration::zero(); }
	void restart() { expiration_ += interval_; }

	void run() {
		timerCallback_();
	}

private:
	const TimerCallback timerCallback_;
	std::chrono::steady_clock::time_point expiration_;
	const std::chrono::steady_clock::duration interval_;
}; // end class Timer

} // end namespace nutty

#endif // NUTTY_BASE_TIMER_H