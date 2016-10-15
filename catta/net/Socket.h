#ifndef CATTA_NET_SOCKET_H
#define CATTA_NET_SOCKET_H

#include <arpa/inet.h>

namespace catta {

class Socket {
public:
	static int create();
	static void close(int sockfd);

public:
	Socket(int sockfd);
	~Socket();

	int fd() const { return sockfd_; }

	int bind(const struct sockaddr_in& addr);
	int listen();
	int accept(struct sockaddr_in* addr);
	int connect(const struct sockaddr_in& addr);

	int shutdownWrite();

	int setReuseAddr(bool reuseaddr);
	int setTcpNoDelay(bool nodelay);
	int setKeepAlive(bool keepalive);
	int setKeepIdle(int optval);

	int getError(int* optval);
	int getSockName(struct sockaddr_in* addr);
	int getPeerName(struct sockaddr_in* addr);

private:
	const int sockfd_;
}; // end class Socket

} // end namespace catta

#endif // CATTA_NET_SOCKET_H