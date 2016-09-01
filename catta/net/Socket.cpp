#include <catta/net/Socket.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>

using namespace catta;

Socket::Socket()
	: sockfd_(::socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC, IPPROTO_TCP)) {
}

Socket::Socket(int sockfd)
	: sockfd_(sockfd) {
}

Socket::~Socket() {
	if (::close(sockfd_) < 0) {
		// FIXME : log
	}
}

int Socket::bind(int sockFd, const struct sockaddr_in &addr) {
	return ::bind(sockFd, reinterpret_cast<const struct sockaddr*>(&addr), static_cast<socklen_t>(sizeof addr));
}

int Socket::listen(int sockFd) {
	return ::listen(sockFd, SOMAXCONN);
}

int Socket::accept(int sockFd, struct sockaddr_in *addr) {
	if (addr != nullptr) {
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
		return ::accept4(sockFd, reinterpret_cast<struct sockaddr*>(addr), &addrlen, SOCK_NONBLOCK|SOCK_CLOEXEC);
	} else {
		return ::accept4(sockFd, nullptr, nullptr, SOCK_NONBLOCK|SOCK_CLOEXEC);
	}
}

int Socket::connect(int sockFd, const struct sockaddr_in &addr) {
	return ::connect(sockFd, reinterpret_cast<const struct sockaddr*>(&addr), static_cast<socklen_t>(sizeof addr));
}

int Socket::shutdownWrite(int sockFd) {
	return ::shutdown(sockFd, SHUT_WR);
}

int Socket::setReuseAddr(int sockFd, bool reuseaddr) {
	int optval = reuseaddr ? 1 : 0;
	return ::setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::setTcpNoDelay(int sockFd, bool nodelay) {
	int optval = nodelay ? 1 : 0;
	return ::setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::setKeepAlive(int sockFd, bool keepalive) {
	int optval = keepalive ? 1 : 0;
	return ::setsockopt(sockFd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::setKeepIdle(int sockFd, int optval) {
	return ::setsockopt(sockFd, SOL_TCP, TCP_KEEPIDLE, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::getError(int sockFd, int *optval) {
	if (optval != nullptr) {
		socklen_t optlen = static_cast<socklen_t>(sizeof *optval);
		return ::getsockopt(sockFd, SOL_SOCKET, SO_ERROR, optval, &optlen);
	} else {
		return ::getsockopt(sockFd, SOL_SOCKET, SO_ERROR, nullptr, nullptr);
	}
}

int Socket::getSockName(int sockFd, struct sockaddr_in *addr) {
	if (addr != nullptr) {
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
		return ::getsockname(sockFd, reinterpret_cast<struct sockaddr*>(addr), &addrlen);
	} else {
		return ::getsockname(sockFd, nullptr, nullptr);
	}
}

int Socket::getPeerName(int sockFd, struct sockaddr_in *addr) {
	if (addr != nullptr) {
		socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
		return ::getpeername(sockFd, reinterpret_cast<struct sockaddr*>(addr), &addrlen);
	} else {
		return ::getpeername(sockFd, nullptr, nullptr);
	}
}
