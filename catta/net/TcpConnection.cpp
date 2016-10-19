#include <catta/net/TcpConnection.h>

#include <catta/net/EventLoop.h>
#include <catta/net/Socket.h>
#include <catta/net/Watcher.h>

using namespace catta;

TcpConnection::TcpConnection(EventLoop* loop, int sockfd,
	const InetAddress& localAddr, const InetAddress& peerAddr)
	: loop_(loop)
	, socket_(new Socket(sockfd))
	, watcher_(new Watcher(loop, sockfd))
	, localAddr_(localAddr)
	, peerAddr_(peerAddr)
	, established_(false)
	, closed_(false) {
	watcher_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	watcher_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	watcher_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	watcher_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {

}

void TcpConnection::send(const void* data, int len) {

}

void TcpConnection::shutdown() {
	if (established_ && !closed_) {
		closed_ = true;
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::forceClose() {
	
}

void TcpConnection::connectEstablished() {
	if (!established_) {
		established_ = true;
		watcher_->setEvents(WatcherEvents::kEventRead);
		watcher_->start();
		connectCallback_(shared_from_this());
	}
}

void TcpConnection::connectDestroyed() {
	if (!closed_) {
		closed_ = true;
		watcher_->stop();
		disconnectCallback_(shared_from_this());
	}
}

void TcpConnection::handleRead() {

}

void TcpConnection::handleWrite() {

}

void TcpConnection::handleClose() {
	closed_ = true;
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

void TcpConnection::shutdownInLoop() {
	if (watcher_->events() | WatcherEvents::kEventWrite) {
		socket_->shutdownWrite();
	}
}