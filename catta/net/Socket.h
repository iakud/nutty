#ifndef CATTA_NET_SOCKET_H
#define CATTA_NET_SOCKET_H

#include <arpa/inet.h>

namespace catta {

class Socket {
public:
	static int create();
	static void close(int sockfd);

public:
	explicit Socket(int sockfd);
	~Socket();

	int fd() const { return sockfd_; }

	int bind(const struct sockaddr_in& addr);
	int listen();
	int accept();
	int accept(struct sockaddr_in& addr);
	int connect(const struct sockaddr_in& addr);

	ssize_t read(void* buf, size_t count);
	ssize_t readv(const struct iovec* iov, int iovcnt);
	ssize_t write(const void* buf, size_t count);
	ssize_t writev(const struct iovec* iov, int iovcnt);

	int shutdownWrite();

	int setReuseAddr(bool reuseaddr);
	int setTcpNoDelay(bool nodelay);
	int setKeepAlive(bool keepalive);
	int setKeepIdle(int optval);

	int getError();
	struct sockaddr_in getSockName();
	struct sockaddr_in getPeerName();

private:
	const int sockfd_;
}; // end class Socket

} // end namespace catta

#endif // CATTA_NET_SOCKET_H