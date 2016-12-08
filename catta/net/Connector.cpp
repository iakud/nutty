#include <catta/net/Connector.h>

#include <catta/net/EventLoop.h>
#include <catta/net/Watcher.h>
#include <catta/net/Socket.h>

using namespace catta;

Connector::Connector(EventLoop* loop, const InetAddress& peerAddr)
	: loop_(loop) {

}


void Connector::connect() {
	loop_->runInLoop(std::bind(&Connector::connectInLoop, this)); // FIXME: unsafe
}

void Connector::reconnect() {

}

void Connector::disconnect() {
	loop_->runInLoop(std::bind(&Connector::disconnectInLoop, this)); // FIXME: unsafe
}

void Connector::retry() {
	if (connect_) {
		// loop_->runInLoop(std::bind(&Connector::connectInLoop, shared_from_this()));
	}
}

void Connector::resetWatcher() {
	watcher_.reset();
}

void Connector::handleWrite() {
	if (connecting_) {
		watcher_->stop();
		watcher_->disableAll();
		loop_->runInLoop(std::bind(&Connector::resetWatcher, this));

		int err = connectSocket_->getError();
		if (err) {
			Socket::close(connectSocket_->fd());

			retry();
		}
	}
}

void Connector::handleError() {
	if (connecting_) {
		watcher_->stop();
		watcher_->disableAll();
		loop_->runInLoop(std::bind(&Connector::resetWatcher, this));

		int err = connectSocket_->getError();
		(void)err;
		// FIXME : err
		Socket::close(connectSocket_->fd());
		connectSocket_.reset();
		connecting_ = false;
		retry();
	}
}

void Connector::connectInLoop() {
	if (connect_) {
		return;
	}
	connect_ = true;
	int sockfd = Socket::create();
	connectSocket_.reset(new Socket(sockfd));
	Socket socket(sockfd);
	int ret = socket.connect(peerAddr_.getSockAddr());
	if (ret == 0) {
		InetAddress localAddr;
		struct sockaddr_in localSockAddr;
		socket.getSockName(&localSockAddr);
		localAddr.setSockAddr(localSockAddr);
		if (connectionCallback_) {
			connectionCallback_(sockfd, localAddr);
		} else {
			Socket::close(sockfd);
		}
		return;
	}

	int err = errno;
	if (err == EINPROGRESS || err == EINTR || err == EISCONN) {
		watcher_.reset(new Watcher(loop_, sockfd));
		watcher_->setWriteCallback(std::bind(&Connector::handleWrite, this));
		watcher_->enableWriting();
		watcher_->start();
		connecting_ = true;
	} else {
		Socket::close(sockfd); // close first
		// retry
		if (err == EAGAIN || err == EADDRINUSE || err == EADDRNOTAVAIL || err == ECONNREFUSED || err == ENETUNREACH) {
			retry();
			//loop_->runInLoop(std::bind(&Connector::connectInLoop, shared_from_this()));
		}
	}
}

void Connector::disconnectInLoop() {
	if (!connect_) {
		return;
	}
	connect_ = false;

}