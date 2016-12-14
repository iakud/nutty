#include <catta/net/Connector.h>

#include <catta/net/EventLoop.h>
#include <catta/net/Watcher.h>
#include <catta/net/Socket.h>

using namespace catta;

Connector::Connector(EventLoop* loop, const InetAddress& peerAddr)
	: loop_(loop)
	, peerAddr_(peerAddr)
	, connect_(false)
	, connecting_(false)
	, retry_(false) {
}

void Connector::start() {
	loop_->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
}

void Connector::restart() {
	if (connect_) {
		if (!connecting_ && !retry_) {
			connect();
		}
	}
}

void Connector::stop() {
	loop_->runInLoop(std::bind(&Connector::stopInLoop, shared_from_this()));
}

void Connector::connect() {
	int sockfd = Socket::create();
	connectSocket_.reset(new Socket(sockfd));
	int ret = connectSocket_->connect(peerAddr_.getSockAddr());
	if (ret == 0) {
		connecting();
		return;
	}

	int err = errno;
	if (ret == 0 || err == EINPROGRESS || err == EINTR || err == EISCONN) {
		connecting();
	} else if (err == EAGAIN || err == EADDRINUSE || err == EADDRNOTAVAIL || err == ECONNREFUSED || err == ENETUNREACH) {
		retry();
	} else {
		Socket::close(sockfd);
		connectSocket_.reset();
	}
}

void Connector::connecting() {
	connecting_ = true;
	watcher_.reset(new Watcher(loop_, connectSocket_->fd()));
	watcher_->setWriteCallback(std::bind(&Connector::handleWrite, this));
	watcher_->setWriteCallback(std::bind(&Connector::handleError, this));
	watcher_->enableWriting();
	watcher_->start();
}

void Connector::stopAndResetWatcher() {
	watcher_->stop();
	watcher_->disableAll();
	loop_->queueInLoop(std::bind(&Connector::resetWatcher, shared_from_this()));
	connecting_ = false;
}

void Connector::resetWatcher() {
	watcher_.reset();
}

void Connector::retry() {
	Socket::close(connectSocket_->fd());
	connectSocket_.reset();
	if (connect_) {
		retry_ = true;
		loop_->runInLoop(std::bind(&Connector::retrying, shared_from_this()));
	}
}

void Connector::retrying() {
	retry_ = false;
	if (connect_) {
		connect();
	}
}

void Connector::handleError() {
	if (connecting_) {
		stopAndResetWatcher();
		int err = connectSocket_->getError();
		(void)err; // FIXME : err
		
		retry();
	}
}

void Connector::handleWrite() {
	if (connecting_) {
		stopAndResetWatcher();
		int err = connectSocket_->getError();
		if (err) {
			retry();
		} else {
			struct sockaddr_in localSockAddr;
			connectSocket_->getSockName(&localSockAddr);
			const struct sockaddr_in& peerSockAddr = peerAddr_.getSockAddr();
			if (localSockAddr.sin_port == peerSockAddr.sin_port && localSockAddr.sin_addr.s_addr == peerSockAddr.sin_addr.s_addr) {
				retry();
			} else {
				int sockfd = connectSocket_->fd();
				connectSocket_.reset();
				if (connectionCallback_) {
					connectionCallback_(sockfd, InetAddress(localSockAddr));
				} else {
					Socket::close(sockfd);
				}
			}
		}
	}
}

void Connector::startInLoop() {
	if (!connect_) {
		connect_ = true;
		if (!retry_) {
			connect();
		}
	}
}

void Connector::stopInLoop() {
	if (connect_) {
		connect_ = false;
		if (connecting_) {
			stopAndResetWatcher();
			Socket::close(connectSocket_->fd());
			connectSocket_.reset();
		}
	}
}