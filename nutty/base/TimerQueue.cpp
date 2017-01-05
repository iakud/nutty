#include <nutty/base/TimerQueue.h>

#include <nutty/base/EventLoop.h>

#include <vector>
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
	, watcher_(loop, timerfd_) {
	watcher_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
	watcher_.enableReading();
	watcher_.start();
}

TimerQueue::~TimerQueue() {
	watcher_.stop();
	::close(timerfd_);
}

void TimerQueue::addTimer(TimerCallback&& cb,
	const std::chrono::steady_clock::time_point& time,
	const std::chrono::steady_clock::duration& interval) {

	TimerPtr timer = std::make_shared<Timer>(std::move(cb), time, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
}

void TimerQueue::addTimerInLoop(TimerPtr& timer) {
	std::multimap<std::chrono::steady_clock::time_point, TimerPtr>::iterator it = timers_.insert(std::make_pair(timer->expiration() , timer));
	if (timers_.begin() == it) {
		//std::cout << "addTimerInLoop 2" << std::endl;
		resetTimer(timer->expiration());
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

	std::multimap<std::chrono::steady_clock::time_point, TimerPtr>::iterator end = timers_.lower_bound(now);
	std::vector<TimerPtr> expired;
	for (auto it = timers_.begin(); it != end; ++it) {
		TimerPtr& timer = it->second;
		timer->run();
		if (timer->repeat()) {
			timer->restart();
			expired.push_back(timer);
		}
	}
	timers_.erase(timers_.begin(), end);
	for (TimerPtr& timer : expired) {
		timers_.insert(std::make_pair(timer->expiration() , timer));
	}
	if (!timers_.empty()) {
		resetTimer(timers_.begin()->first);
	} else {
		//std::chrono::steady_clock::time_point zero(std::chrono::steady_clock::duration::zero());
		//resetTimer(zero);
	}

	//std::transform(timers_.begin(), it, expired.begin(), std::get<1, std::chrono::steady_clock::time_point, std::shared_ptr<Timer>>);
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