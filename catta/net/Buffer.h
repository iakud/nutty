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

	void clear();
	void reset();

private:
	static const size_t kCapacity = 1024 * 8;

	char buffer_[kCapacity];
	size_t capacity_;
	size_t write_;
	size_t read_;
	Buffer* next_;

	friend class BufferPool;
	friend class SendBuffer;
	friend class ReceiveBuffer;
}; // end class Buffer

class BufferPool : noncopyable {
public:
	BufferPool(uint32_t capacity);
	~BufferPool();

	void put(Buffer* buffer);
	Buffer* take();

	Buffer* next() {
		if (!next_) {
			next_ = take();
		}
		return next_;
	}

private:
	const uint32_t capacity_;
	uint32_t count_;
	Buffer* head_;		// head buffer
	Buffer* tail_;		// tail buffer
	Buffer* next_;
}; // end class BufferPool

class SendBuffer : noncopyable {
public:
	SendBuffer(BufferPool* pool);
	~SendBuffer();

	void write(const char* buf, size_t count);

private:
	ssize_t writeSocket(Socket& socket);

	BufferPool* pool_;
	size_t count_;
	Buffer* head_;
	Buffer* tail_;

	friend class TcpConnection;
}; // end class SendBuffer

class ReceiveBuffer : noncopyable {
public:
	ReceiveBuffer(BufferPool* pool);
	~ReceiveBuffer();

	void peek(char* buf, size_t count);
	void read(char* buf, size_t count);

private:
	ssize_t readSocket(Socket& socket);

	BufferPool* pool_;
	size_t count_;
	Buffer* head_;
	Buffer* tail_;

	friend class TcpConnection;
}; // end class ReceiveBuffer

} // end namespace catta

#endif // CATTA_NET_BUFFER_H