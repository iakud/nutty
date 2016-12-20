#ifndef CATTA_NET_INETADDRESS_H
#define CATTA_NET_INETADDRESS_H

#include <cstdint>

#include <arpa/inet.h>

namespace catta {

class InetAddress {
public:
	InetAddress() {
	}

	InetAddress(const char* ip, uint16_t port);

	InetAddress(const struct sockaddr_in& addr)
		: addr_(addr) {
	}

	const struct sockaddr_in& getSockAddr() const {
		return addr_;
	}

	void setSockAddr(const struct sockaddr_in& addr) {
		addr_ = addr;
	}

private:
	struct sockaddr_in addr_;
}; // end class InetAddress

} // end namespace catta

#endif // CATTA_NET_INETADDRESS_H