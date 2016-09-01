#ifndef CATTA_NET_SOCKET_H
#define CATTA_NET_SOCKET_H

namespace catta {

class Socket {
public:
	Socket();
	Socket(int sockfd);
	~Socket();

	int fd() const { return sockfd_; }

	
private:
	const int sockfd_;
}; // end class Socket

} // end namespace catta

#endif // CATTA_NET_SOCKET_H