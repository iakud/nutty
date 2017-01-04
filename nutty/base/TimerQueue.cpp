#include <nutty/base/TimerQueue.h>

#include <nutty/base/EventLoop.h>

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
	const std::chrono::steady_clock::time_point& when,
	const std::chrono::steady_clock::duration& interval) {

	Timer* timer = new Timer(std::move(cb), when, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
	//return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer* timer) {
	/*bool earliestChanged = insert(timer);
	if (earliestChanged) {
		resetTimerfd(timerfd_, timer->expiration());
	}*/
}

void TimerQueue::handleRead() {
	uint64_t howmany;
	ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
	//LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
	if (n != sizeof howmany) {
		// LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
	}
}

void TimerQueue::resetTimer(const std::chrono::steady_clock::time_point& expiration) {
	struct itimerspec utmr;
	std::chrono::steady_clock::duration dtn = std::chrono::duration_cast<std::chrono::milliseconds>(expiration.time_since_epoch());
	utmr.it_value.tv_sec = dtn.count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;
	//utmr.it_value.tv_nsec = dtn.count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den
	int ret = ::timerfd_settime(timerfd_, 1, &utmr, nullptr);
	if (ret) {
		// FIXME : log
	}
}