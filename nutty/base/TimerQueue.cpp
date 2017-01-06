#include <nutty/base/TimerQueue.h>

#include <nutty/base/Timer.h>
#include <nutty/base/EventLoop.h>

#include <vector>
#include <algorithm>
#include <sys/timerfd.h>

namespace {

int createTimerfd() {
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0) {
		::abort(); // FIXME log
	}
	return timerfd;
}

} // end anonymous namespace

using namespace nutty;

TimerQueue::TimerQueue(EventLoop* loop)
	: loop_(loop)
	, timerfd_(createTimerfd())
	, watcher_(loop, timerfd_)
	, callingExpiredTimers_(false) {
	watcher_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
	watcher_.enableReading();
	watcher_.start();
}

TimerQueue::~TimerQueue() {
	watcher_.stop();
	::close(timerfd_);
}

TimerPtr TimerQueue::addTimer(TimerCallback&& cb,
	const std::chrono::steady_clock::time_point& time,
	const std::chrono::steady_clock::duration& interval) {
	TimerPtr timer = std::make_shared<Timer>(std::move(cb), time, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
	return timer;
}

void TimerQueue::cancel(TimerPtr& timer) {
	loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timer));
}

void TimerQueue::addTimerInLoop(TimerPtr& timer) {
	TimerMap::iterator it = timers_.insert(std::make_pair(timer->expiration() , timer));
	if (timers_.begin() == it) {
		resetTimer(timer->expiration());
	}
}

void TimerQueue::cancelInLoop(TimerPtr& timer) {
	std::pair<TimerMap::iterator, TimerMap::iterator> range = timers_.equal_range(timer->expiration());
	TimerMap::iterator it = std::find(range.first, range.second, TimerMap::value_type(timer->expiration(), timer));
	if (it != range.second) {
		timers_.erase(it);
	} else if (callingExpiredTimers_) {
		cancelingTimers_.insert(timer);
	}
}

void TimerQueue::handleRead() {
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	uint64_t howmany;
	ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
	//LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
	if (n != sizeof howmany) {
		// LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
	}

	TimerMap::iterator bound = timers_.upper_bound(now);
	std::vector<TimerPtr> expired;
	for (TimerMap::iterator it = timers_.begin(); it != bound; ++it) {
		expired.push_back(it->second);
	}
	timers_.erase(timers_.begin(), bound);

	callingExpiredTimers_ = true;
	cancelingTimers_.clear();
	for (TimerPtr& timer : expired) {
		timer->run();
	}
	callingExpiredTimers_ = false;

	for (TimerPtr& timer : expired) {
		if (timer->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
			timer->restart();
			timers_.insert(std::make_pair(timer->expiration(), timer));
		}
	}
	// reset timer
	if (!timers_.empty()) {
		resetTimer(timers_.begin()->first);
	} else {
		resetTimer(std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration::zero()));
	}
}

void TimerQueue::resetTimer(const std::chrono::steady_clock::time_point& time) {
	struct itimerspec utmr, otmr;
	std::chrono::steady_clock::duration sinceEpoch = time.time_since_epoch();
	std::chrono::seconds secs = std::chrono::duration_cast<std::chrono::seconds>(sinceEpoch);
	std::chrono::nanoseconds nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(sinceEpoch);
	utmr.it_value.tv_sec = secs.count();
	utmr.it_value.tv_nsec = (nsecs - secs).count();
	int ret = ::timerfd_settime(timerfd_, TFD_TIMER_ABSTIME, &utmr, &otmr);
	if (ret) {
		// FIXME : log
	}
}