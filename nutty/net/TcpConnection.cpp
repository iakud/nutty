#include <nutty/net/TcpConnection.h>

#include <nutty/net/Socket.h>
#include <nutty/base/Watcher.h>
#include <nutty/base/EventLoop.h>

using namespace nutty;

void defaultConnectCallback(const TcpConnectionPtr& connection) {

}

void defaultReadCallback(const TcpConnectionPtr& connection, ReceiveBuffer& buffer) {
	// buffer->retrieveAll();
}

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
	watcher_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	watcher_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
	watcher_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	watcher_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
	Socket::close(socket_->fd());
}

void TcpConnection::setTcpNoDelay(bool nodelay) {
	socket_->setTcpNoDelay(nodelay);
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

void TcpConnection::send(const std::string& data) {
	if (state_ == kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop(data.c_str(), static_cast<uint32_t>(data.size()));
		} else {
			void (TcpConnection::*fp)(BufferPtr&) = &TcpConnection::sendInLoop;
			loop_->queueInLoop(std::bind(fp, this, std::make_shared<Buffer>(data.c_str(), data.size())));
		}
	}
}

void TcpConnection::send(ReceiveBuffer& buffer) {

}

void TcpConnection::shutdown() {
	loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
}

void TcpConnection::forceClose() {
	loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
}

void TcpConnection::established() {
	loop_->runInLoop(std::bind(&TcpConnection::establishedInLoop, shared_from_this()));
}

void TcpConnection::destroyed() {
	loop_->queueInLoop(std::bind(&TcpConnection::destroyedInLoop, shared_from_this()));
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

void TcpConnection::handleRead() {
	struct iovec iov[2];
	int iovcnt = 0;
	receiveBuffer_.prepareReceive(iov, iovcnt);
	ssize_t nread = socket_->readv(iov, iovcnt);
	if (nread > 0) {
		receiveBuffer_.hasReceived(static_cast<uint32_t>(nread));
		readCallback_(shared_from_this(), receiveBuffer_);
	} else if (nread == 0) {
		handleClose();
	} else {
		handleError();
	}
}

void TcpConnection::handleWrite() {
	if (watcher_->isWriting()) {
		struct iovec iov[sendBuffer_.buffersSize()];
		int iovcnt;
		sendBuffer_.prepareSend(iov, iovcnt);
		ssize_t nwrote = socket_->writev(iov, iovcnt);
		if (nwrote > 0) {
			sendBuffer_.hasSent(static_cast<uint32_t>(nwrote));
			if (sendBuffer_.size() == 0) {
				watcher_->disableWriting();
				if (writeCallback_) {
					// loop_->queueInLoop(std::bind(writeCallback_, shared_from_this(), nwrote));
				}
				if (state_ == kDisconnecting) {
					shutdownInLoop();
				}
			}
		} else if (nwrote == 0) {
			// do nothing
		} else {
			// FIXME : log
		}
	}
}

void TcpConnection::sendInLoop(const void* buf, uint32_t count) {
	if (state_ == kDisconnected) {
		// FIXME : log
		return;
	}
	if (!watcher_->isWriting() && count > 0) {
		ssize_t nwrote = socket_->write(buf, count);
		if (nwrote > 0) {
			if (nwrote < count) {
				sendBuffer_.append(static_cast<const char*>(buf) + nwrote, count - static_cast<uint32_t>(nwrote));
				watcher_->enableWriting();
			} else if (writeCallback_) {
				// loop_->queueInLoop(std::bind(writeCallback_, shared_from_this(), nwrote));
			}
		} else if (nwrote == 0) {
			sendBuffer_.append(buf, count);
			watcher_->enableWriting();
		} else {
			if (errno != EWOULDBLOCK) {
				// FIXME log
				if (errno == EPIPE || errno == ECONNRESET) {
					return;
				}
			}
			sendBuffer_.append(buf, count);
			watcher_->enableWriting();
		}
	}
}

void TcpConnection::sendInLoop(BufferPtr& buf) {
	if (state_ == kDisconnected) {
		// FIXME : log
		return;
	}
	uint32_t count = buf->readableSize();
	if (!watcher_->isWriting() && count > 0) {
		ssize_t nwrote = socket_->write(buf->data(), count);
		if (nwrote > 0) {
			if (nwrote < count) {
				buf->hasRead(static_cast<uint32_t>(nwrote));
				sendBuffer_.append(std::move(*buf));
				watcher_->enableWriting();
			} else if (writeCallback_) {
				// loop_->queueInLoop(std::bind(writeCallback_, shared_from_this(), nwrote));
			}
		} else if (nwrote == 0) {
			sendBuffer_.append(std::move(*buf));
			watcher_->enableWriting();
		} else {
			if (errno != EWOULDBLOCK) {
				// FIXME log
				if (errno == EPIPE || errno == ECONNRESET) {
					return;
				}
			}
			sendBuffer_.append(std::move(*buf));
			watcher_->enableWriting();
		}
	}
}

void TcpConnection::shutdownInLoop() {
	if (state_ == kConnected) {
		setState(kDisconnecting);
		if (!watcher_->isWriting()) {
			socket_->shutdownWrite();
		}
	}
}

void TcpConnection::forceCloseInLoop() {
	if (state_ == kConnected || state_ == kDisconnecting) {
		handleClose();
	}
}

void TcpConnection::establishedInLoop() {
	if (state_ == kConnecting) {
		setState(kConnected);
		watcher_->enableReading();
		watcher_->start();
		connectCallback_(shared_from_this());
	}
}

void TcpConnection::destroyedInLoop() {
	if (state_ == kConnected) {
		setState(kDisconnected);
		watcher_->stop();
		disconnectCallback_(shared_from_this());
	}
}