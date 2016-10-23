#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

#include <unistd.h>

namespace catta {

class Socket;

class Buffer {
public:
	Buffer();
	~Buffer();

	bool empty() { return count_ == 0; }

private:
	static const size_t kCapacity = 1024 * 8;

	char buffer_[kCapacity];
	size_t capacity_;
	size_t count_;
	size_t write_;
	size_t read_;
	Buffer* next_;

	friend class ListBuffer;
}; // end class Buffer

class ListBuffer {
public:
	ListBuffer();

	ssize_t write(Socket& socket);
	ssize_t read(Socket& socket);
private:
	ssize_t readv(Socket& socket);

	size_t count_;
	Buffer* head_;
	Buffer* current_;
	Buffer* next_;
}; // end class ListBuffer

class BufferPool {
public:

private:

}; // end class BufferPool

} // end namespace catta

#endif // CATTA_NET_BUFFER_H