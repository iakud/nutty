#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

#include <catta/base/noncopyable.h>

#include <cstdint>

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

class BufferPool : noncopyable {
public:
	BufferPool(const uint32_t size)
		: size_(size)
		, count_(0) {
	}
	~BufferPool() {
	}

	void put(Buffer* buffer) {
		if (buffer) {
			delete buffer;
		}
	}

	Buffer* take() {
		Buffer* buffer = new Buffer();
		return buffer;
	}

private:
	const uint32_t size_;
	uint32_t count_;
	Buffer* head_;		// head buffer
	Buffer* tail_;		// tail buffer
}; // end class BufferPool

class ListBuffer {
public:
	ListBuffer();

	ssize_t write(Socket& socket);
	ssize_t read(Socket& socket);

private:
	BufferPool* pool_;
	size_t count_;
	Buffer* head_;
	Buffer* current_;
	Buffer* next_;
}; // end class ListBuffer

} // end namespace catta

#endif // CATTA_NET_BUFFER_H