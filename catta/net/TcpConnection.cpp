#include <catta/net/TcpConnection.h>

#include <catta/net/EventLoop.h>
#include <catta/net/Socket.h>
#include <catta/net/Watcher.h>

using namespace catta;

TcpConnection::TcpConnection(EventLoop* loop, int sockfd,
	const InetAddress& localAddr, const InetAddress& peerAddr)
	: loop_(loop)
	, state_(kConnecting)
	, socket_(new Socket(sockfd))
	, watcher_(new Watcher(loop, sockfd))
	, localAddr_(localAddr)
	, peerAddr_(peerAddr) {
	watcher_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	watcher_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	watcher_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	watcher_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {

}

void TcpConnection::send(const void* data, int len) {
	if (state_ == kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop();
		} else {
			loop_->queueInLoop(std::bind(&TcpConnection::sendInLoop, this));
		}
	}
}

void TcpConnection::shutdown() {
	if (state_ == kConnected) {
		setState(kDisconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::forceClose() {
	loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
}

void TcpConnection::connectEstablished() {
	if (state_ == kConnecting) {
		setState(kConnected);
		watcher_->setEvents(WatcherEvents::kEventRead);
		watcher_->start();
		connectCallback_(shared_from_this());
	}
}

void TcpConnection::connectDestroyed() {
	if (state_ == kConnected) {
		setState(kDisconnected);
		watcher_->stop();
		disconnectCallback_(shared_from_this());
	}
}

void TcpConnection::handleRead() {

}

void TcpConnection::handleWrite() {

}

void TcpConnection::handleClose() {
	setState(kDisconnected);
	watcher_->stop();
	TcpConnectionPtr guardThis(shared_from_this());
	disconnectCallback_(guardThis);
	closeCallback_(guardThis);
}

void TcpConnection::handleError() {
	int err = socket_->getError();
	(void)err;
	// FIXME :
	// LOG_ERROR
}

void TcpConnection::sendInLoop() {

}

void TcpConnection::shutdownInLoop() {
	if (watcher_->events() | WatcherEvents::kEventWrite) {
		socket_->shutdownWrite();
	}
}

void TcpConnection::forceCloseInLoop() {
	if (state_ == kConnected || state_ == kDisconnecting) {
		handleClose();
	}
}