#include "Watcher.h"

#include "EventLoop.h"

using namespace catta;

Watcher::Watcher(const int fd, EventLoop* loop)
	: fd_(fd)
	, loop_(loop)
	, events_(WatcherEvents::kEventNone)
	, revents_(WatcherEvents::kEventNone)
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
	if (revents_ & kEventClose) {
		if (!closeCallback_ || !closeCallback_()) {
			revents_ &= ~kEventClose;
		}
	}
	if (revents_ & kEventError) {
		if (!errorCallback_ || !errorCallback_()) {
			revents_ &= ~kEventError;
		}
	}
	if (revents_ & kEventRead) {
		if (!readCallback_ || !readCallback_()) {
			revents_ &= ~kEventRead;
		}
	}
	if (revents_ & kEventWrite) {
		if (!writeCallback_ || !writeCallback_()) {
			revents_ &= ~kEventWrite;
		}
	}
}