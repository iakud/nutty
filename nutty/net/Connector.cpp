#include <nutty/net/Connector.h>

#include <nutty/base/EventLoop.h>
#include <nutty/base/Watcher.h>
#include <nutty/net/Socket.h>

using namespace nutty;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& peerAddr)
	: loop_(loop)
	, peerAddr_(peerAddr)
	, connect_(false)
	, connecting_(false)
	, retry_(false)
	, retryDelayMs_(kInitRetryDelayMs) {
}

Connector::~Connector() {
}

void Connector::start() {
	loop_->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
}

void Connector::restart() {
	if (connect_) {
		if (!connecting_ && !retry_) {
			retryDelayMs_ = kInitRetryDelayMs;
			connect();
		}
	}
}

void Connector::stop() {
	loop_->runInLoop(std::bind(&Connector::stopInLoop, shared_from_this()));
}

void Connector::connect() {
	connectSocket_.reset(new Socket());
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
		connectSocket_.reset();
	}
}

void Connector::connecting() {
	connecting_ = true;
	watcher_.reset(new Watcher(loop_, connectSocket_->fd()));
	watcher_->setErrorCallback(std::bind(&Connector::handleError, this));
	watcher_->setWriteCallback(std::bind(&Connector::handleWrite, this));
	watcher_->enableWriting();
}

void Connector::stopAndResetWatcher() {
	watcher_->disableAll();
	loop_->queueInLoop(std::bind(&Connector::resetWatcher, shared_from_this()));
	connecting_ = false;
}

void Connector::resetWatcher() {
	watcher_.reset();
}

void Connector::retry() {
	connectSocket_.reset();
	if (connect_) {
		retry_ = true;
		loop_->runAfter(std::chrono::milliseconds(retryDelayMs_), std::bind(&Connector::retrying, shared_from_this()));
		retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
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
			struct sockaddr_in localSockAddr = connectSocket_->getSockName();
			const struct sockaddr_in& peerSockAddr = peerAddr_.getSockAddr();
			if (localSockAddr.sin_port == peerSockAddr.sin_port && localSockAddr.sin_addr.s_addr == peerSockAddr.sin_addr.s_addr) {
				retry();
			} else {
				Socket socket(std::move(*connectSocket_.get()));
				connectSocket_.reset();
				if (connectionCallback_) {
					connectionCallback_(std::move(socket), InetAddress(localSockAddr));
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
			connectSocket_.reset();
		}
	}
}