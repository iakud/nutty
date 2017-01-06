#include <nutty/base/Timer.h>

using namespace nutty;

Timer::Timer(TimerCallback&& cb,
	const std::chrono::steady_clock::time_point& time,
	const std::chrono::steady_clock::duration& interval)
	: timerCallback_(std::move(cb))
	, expiration_(time)
	, interval_(interval) {
}

void Timer::restart() {
	expiration_ += interval_;
}