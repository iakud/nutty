#include <catta/net/InetAddress.h>

#include <string.h>

using namespace catta;

InetAddress::InetAddress(const char* ip, uint16_t port) {
	::bzero(&addr_, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port);
	::inet_pton(AF_INET, ip, &addr_.sin_addr);
}