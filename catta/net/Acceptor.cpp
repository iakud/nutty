#include <catta/net/Acceptor.h>

#include "Socket.h"
#include <catta/net/EventLoop.h>
#include <catta/net/Watcher.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace catta;

Acceptor::Acceptor(EventLoop* loop, const struct sockaddr_in& localSockAddr)
	: loop_(loop)
	, localSockAddr_(localSockAddr)
	, sockFd_(Socket::open())
	, watcher_(loop, sockFd_)
	, listenning_(false)
	, idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
	Socket::bind(sockFd_, localSockAddr_);
	Socket::setReuseAddr(sockFd_, true);
	watcher_.setReadCallback(std::bind(&Acceptor::handleRead, this));
	watcher_.setEvents(WatcherEvents::kEventRead);
}

Acceptor::~Acceptor() {
	Socket::close(sockFd_);
	::close(idleFd_);
	watcher_.stop();
}

void Acceptor::listen() {
	listenning_ = true;
	Socket::listen(sockFd_);
	watcher_.start();
}

bool Acceptor::handleRead() {
	if (!accept_) {
		return;
	}
	struct sockaddr_in peerSockAddr;
	int sockFd = Socket::accept(sockFd_, &peerSockAddr);
	if (sockFd < 0) {
		int err = errno;// on error
		if (EAGAIN == err) {

		} else if (EMFILE == err || ENFILE == err) {
			::close(idleFd_);
			idleFd_ = Socket::accept(sockFd_, nullptr);
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);

			watcher_->activeEvents(WatcherEvents::kEventRead);
		} else {
			// FIXME
		}
	} else {
		// accept successful
		if (acceptCallback_) {
			acceptCallback_(sockFd, peerSockAddr);
		} else {
			Socket::close(sockFd);
		}

		watcher_->activeEvents(WatcherEvents::kEventRead);
	}
}