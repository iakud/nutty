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
	, peerAddr_(peerAddr)
	, sendBuffer_()
	, receiveBuffer_() {
	watcher_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	watcher_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	watcher_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	watcher_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
	
}

void TcpConnection::send(const void* buf, uint32_t count) {
	if (state_ == kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop(buf, count);
		} else {
			void (TcpConnection::*fp)(BufferPtr&) = &TcpConnection::sendInLoop;
			loop_->queueInLoop(std::bind(fp, this, std::make_shared<Buffer>(buf, count)));
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
	//struct iovec* iov;
	//int iovcnt;
	//receiveBuffer_.prepareReceive(&iov, &iovcnt);
	//ssize_t nread = socket_->readv(iov, iovcnt);
	//if (nread > 0) {
	//	receiveBuffer_.hasReceived(static_cast<uint32_t>(nread));
	//}
}

void TcpConnection::handleWrite() {
	if (writable_) {
		return;
	}
	//struct iovec* iov;
	//int iovcnt;
	//sendBuffer_.prepareSend(&iov, &iovcnt);
	//ssize_t nwrote = socket_->writev(iov, iovcnt);
	//if (nwrote > 0) {
	//	sendBuffer_.hasSent(static_cast<uint32_t>(nwrote));
	//	if (sendBuffer_.size() == 0) {

	//	}
	//} else {
		// FIXME : log
	//}
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

void TcpConnection::sendInLoop(const void* buf, uint32_t count) {
	if (state_ == kDisconnected) {
		// FIXME : log
		return;
	}
	if (writable_) {
		ssize_t nwrote = socket_->write(buf, count);
		if (nwrote < 0) {
			if (errno != EWOULDBLOCK) {
				if (errno == EPIPE || errno == ECONNRESET) {
					return;
				}
			}
			sendBuffer_.append(buf, count);
			writable_ = false;
		} else {
			sendBuffer_.append(static_cast<const char*>(buf) + nwrote, count - static_cast<uint32_t>(nwrote));
			writable_ = false;
		}
	}
}

void TcpConnection::sendInLoop(BufferPtr& buffer) {
	if (state_ == kDisconnected) {
		// FIXME : log
		return;
	}
	if (writable_) {
		ssize_t nwrote = socket_->write(buffer->data(), buffer->size());
		if (nwrote < 0) {
			if (errno != EWOULDBLOCK) {
				if (errno == EPIPE || errno == ECONNRESET) {
					return;
				}
			}
			sendBuffer_.append(std::move(*buffer));
			writable_ = false;
		} else {
			sendBuffer_.append(std::move(*buffer), static_cast<uint32_t>(nwrote));
			writable_ = false;
		}
	}
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