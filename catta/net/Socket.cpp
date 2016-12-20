#include <catta/net/Socket.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>  // bzero

using namespace catta;

int Socket::create() {
	int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if (sockfd < 0) {
		// FIXME : log
		// LOG_FATAL
	}
	return sockfd;
}

void Socket::close(int sockfd) {
	if (::close(sockfd) < 0) {
		// FIXME : log
		// LOG_ERR
	}
}

Socket::Socket(int sockfd)
	: sockfd_(sockfd) {
}

Socket::~Socket() {
}

int Socket::bind(const struct sockaddr_in& addr) {
	return ::bind(sockfd_, reinterpret_cast<const struct sockaddr*>(&addr), static_cast<socklen_t>(sizeof addr));
}

int Socket::listen() {
	return ::listen(sockfd_, SOMAXCONN);
}

int Socket::accept() {
	return ::accept4(sockfd_, nullptr, nullptr, SOCK_NONBLOCK|SOCK_CLOEXEC);
}

int Socket::accept(struct sockaddr_in& addr) {
	socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
	return ::accept4(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), &addrlen, SOCK_NONBLOCK|SOCK_CLOEXEC);
}

int Socket::connect(const struct sockaddr_in& addr) {
	return ::connect(sockfd_, reinterpret_cast<const struct sockaddr*>(&addr), static_cast<socklen_t>(sizeof addr));
}

ssize_t Socket::read(void* buf, size_t count) {
	return ::read(sockfd_, buf, count);
}

ssize_t Socket::readv(const struct iovec* iov, int iovcnt) {
	return ::readv(sockfd_, iov, iovcnt);
}

ssize_t Socket::write(const void* buf, size_t count) {
	return ::write(sockfd_, buf, count);
}

ssize_t Socket::writev(const struct iovec* iov, int iovcnt) {
	return ::writev(sockfd_, iov, iovcnt);
}

int Socket::shutdownWrite() {
	return ::shutdown(sockfd_, SHUT_WR);
}

int Socket::setReuseAddr(bool reuseaddr) {
	int optval = reuseaddr ? 1 : 0;
	return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::setTcpNoDelay(bool nodelay) {
	int optval = nodelay ? 1 : 0;
	return ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::setKeepAlive(bool keepalive) {
	int optval = keepalive ? 1 : 0;
	return ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::setKeepIdle(int optval) {
	return ::setsockopt(sockfd_, SOL_TCP, TCP_KEEPIDLE, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::getError() {
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
		return errno;
	} else {
		return optval;
	}
}

struct sockaddr_in Socket::getSockName() {
	struct sockaddr_in addr;
	socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
	if (::getsockname(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), &addrlen) < 0) {
		bzero(&addr, sizeof addr);
	}
	return addr;
}

struct sockaddr_in Socket::getPeerName() {
	struct sockaddr_in addr;
	socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
	if (::getpeername(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), &addrlen) < 0) {
		bzero(&addr, sizeof addr);
	}
	return addr;
}
