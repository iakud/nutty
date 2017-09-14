#include <nutty/base/Watcher.h>

#include <nutty/base/EventLoop.h>

#include <poll.h>

using namespace nutty;

const int Watcher::kNoneEvent = 0;
const int Watcher::kReadEvent = POLLIN | POLLPRI;
const int Watcher::kWriteEvent = POLLOUT;

Watcher::Watcher(EventLoop* loop, const int fd)
	: loop_(loop)
	, fd_(fd)
	, events_(0)
	, revents_(0)
	, started_(false) {
}

Watcher::~Watcher() {
}

void Watcher::update() {
	if (started_) {
		if (events_ == kNoneEvent) {
			loop_->removeWatcher(this);
			started_ = false;
		} else {
			loop_->updateWatcher(this);
		}
	} else {
		if (events_ != kNoneEvent) {
			loop_->addWatcher(this);
			started_ = true;
		}
	}
}

void Watcher::handleEvents() {
	if (revents_ & POLLHUP && !(revents_ & POLLIN)) {
		if (closeCallback_) {
			closeCallback_();
		}
	}
	if (revents_ & POLLNVAL) {
		// FIXME
	}
	if (revents_ & (POLLERR | POLLNVAL)) {
		if (errorCallback_) {
			errorCallback_();
		}
	}
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
		if (readCallback_) {
			readCallback_();
		}
	}
	if (revents_ & POLLOUT) {
		if (writeCallback_) {
			writeCallback_();
		}
	}
}