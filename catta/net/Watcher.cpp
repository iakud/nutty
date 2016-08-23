#include "Watcher.h"

#include "EventLoop.h"

using namespace catta;

Channel::Channel(EventLoop* loop, const int fd)
	: loop_(loop)
	, fd_(fd)
	, read_(false)
	, write_(false)
	, rclose_(false)
	, rerror_(false)
	, rread_(false)
	, rwrite_(false)
	, started_(false)
	, actived_(false)
	, readable_(false)
	, writeable_(false) {
}

Channel::~Channel() {
}

void Channel::onClose() {
	if (!rclose_) {
		rclose_ = true;
		addActivedChannel();
	}
}

void Channel::onError() {
	if (!rerror_) {
		rerror_ = true;
		addActivedChannel();
	}
}

void Channel::onRead() {
	if (read_ && !readable_ && !rread_) {
		readable_ = rread_ = true;
		addActivedChannel();
	}
}

void Channel::onWrite() {
	if (write_ && !writeable_ && !rwrite_) {
		writeable_ = rwrite_ = true;
		addActivedChannel();
	}
}

void Channel::handleEvents() {
	actived_ = false;
	if (rclose_) {
		rclose_ = false;
		if (closeCallback_) {
			closeCallback_();
		}
	}
	if (rerror_) {
		rerror_ = false;
		if (errorCallback_) {
			errorCallback_();
		}
	}
	if (rread_) {
		rread_ = false;
		if (readCallback_) {
			readCallback_();
		}
	}
	if (rwrite_) {
		rwrite_ = false;
		if (writeCallback_) {
			writeCallback_();
		}
	}
}