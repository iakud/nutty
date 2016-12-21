#include <nutty/base/Watcher.h>

#include <nutty/base/EventLoop.h>

#include <sys/epoll.h>

using namespace nutty;

const int Watcher::kNoneEvent = 0;
const int Watcher::kReadEvent = EPOLLIN | EPOLLPRI;
const int Watcher::kWriteEvent = EPOLLOUT;

Watcher::Watcher(EventLoop* loop, const int fd)
	: loop_(loop)
	, fd_(fd)
	, events_(0)
	, revents_(0)
	, started_(false) {
}

Watcher::~Watcher() {
}

void Watcher::start() {
	if (!started_) {
		loop_->addWatcher(this);
		started_ = true;
	}
}

void Watcher::update() {
	if (started_) {
		loop_->updateWatcher(this);
	}
}

void Watcher::stop() {
	if (started_) {
		loop_->removeWatcher(this);
		started_ = false;
	}
}

void Watcher::handleEvents() {
	if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
		if (closeCallback_) {
			closeCallback_();
		}
	}
	if (revents_ & EPOLLERR) {
		if (errorCallback_) {
			errorCallback_();
		}
	}
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
		if (readCallback_) {
			readCallback_();
		}
	}
	if (revents_ & EPOLLOUT) {
		if (writeCallback_) {
			writeCallback_();
		}
	}
}