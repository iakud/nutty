#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

namespace catta {

class Buffer {
public:

}; // end class Buffer

class SendBuffer {
public:
	ssize_t write(Socket& socket);
}; // end class SendBuffer

class ReceiveBuffer {
public:
	ssize_t read(Socket& socket);
}; // end class ReceiveBuffer

} // end namespace catta

#endif // CATTA_NET_BUFFER_H