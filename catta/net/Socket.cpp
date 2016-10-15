#include <catta/net/Socket.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>

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
	if (::close(sockfd_) < 0) {
		// FIXME : log
		// LOG_ERR
	}
}

int Socket::bind(const struct sockaddr_in& addr) {
	return ::bind(sockfd_, reinterpret_cast<const struct sockaddr*>(&addr), static_cast<socklen_t>(sizeof addr));
}

int Socket::listen() {
	return ::listen(sockfd_, SOMAXCONN);
}

int Socket::accept(struct sockaddr_in* addr) {
	if (addr != nullptr) {
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
		return ::accept4(sockfd_, reinterpret_cast<struct sockaddr*>(addr), &addrlen, SOCK_NONBLOCK|SOCK_CLOEXEC);
	} else {
		return ::accept4(sockfd_, nullptr, nullptr, SOCK_NONBLOCK|SOCK_CLOEXEC);
	}
}

int Socket::connect(const struct sockaddr_in& addr) {
	return ::connect(sockfd_, reinterpret_cast<const struct sockaddr*>(&addr), static_cast<socklen_t>(sizeof addr));
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

int Socket::getError(int* optval) {
	if (optval != nullptr) {
		socklen_t optlen = static_cast<socklen_t>(sizeof *optval);
		return ::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, optval, &optlen);
	} else {
		return ::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, nullptr, nullptr);
	}
}

int Socket::getSockName(struct sockaddr_in* addr) {
	if (addr != nullptr) {
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
		return ::getsockname(sockfd_, reinterpret_cast<struct sockaddr*>(addr), &addrlen);
	} else {
		return ::getsockname(sockfd_, nullptr, nullptr);
	}
}

int Socket::getPeerName(struct sockaddr_in* addr) {
	if (addr != nullptr) {
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
		return ::getpeername(sockfd_, reinterpret_cast<struct sockaddr*>(addr), &addrlen);
	} else {
		return ::getpeername(sockfd_, nullptr, nullptr);
	}
}
