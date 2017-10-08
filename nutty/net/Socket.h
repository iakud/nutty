#ifndef NUTTY_NET_SOCKET_H
#define NUTTY_NET_SOCKET_H

#include <arpa/inet.h>

namespace nutty {

class Socket {
public:
	Socket();
	Socket(Socket&& socket);
	~Socket();

	int fd() const { return sockfd_; }

	int bind(const struct sockaddr_in& addr);
	int listen();
	Socket accept();
	Socket accept(struct sockaddr_in& addr);
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
	explicit Socket(int sockfd);

	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	int sockfd_;
}; // end class Socket

} // end namespace nutty

#endif // NUTTY_NET_SOCKET_H