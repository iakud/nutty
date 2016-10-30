#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

#include <catta/base/noncopyable.h>

#include <cstdint>
#include <memory>
#include <unistd.h>

struct iovec;
class Socket;

namespace catta {
/*
class Buffer {
public:
	Buffer(const char* buf, uint32_t count);
	Buffer(Buffer&& buffer);
	~Buffer();

	Buffer& operator=(Buffer&& other) {
		return *this;
	}

	const char* data() { return buffer_; }
	uint32_t size() { return count_; }

private:
	char* buffer_;
	uint32_t count_;
	bool create_; // FIXME

	friend class LinkedBuffer;
}; // end class Buffer

typedef std::shared_ptr<Buffer> BufferPtr;

class LinkedBuffer : noncopyable {
public:
	LinkedBuffer(const char* buf, uint32_t count);
	LinkedBuffer(Buffer&& buffer);
	~LinkedBuffer();

	void retrieve(uint32_t count) {
		if (count < write_ - read_) {
			read_ += count;
		} else {
			write_ = read_ = 0;
		}
	}

private:
	char* buffer_;
	uint32_t capacity_;
	uint32_t write_;
	uint32_t read_;
	LinkedBuffer* next_;

	friend class SendBuffer;
};

class LinkedBufferList : noncopyable {
public:
	LinkedBufferList();

	LinkedBuffer* head() { return head_; }
	LinkedBuffer* tail() { return tail_; }

	void push(LinkedBuffer* buffer);
	void pop();

	bool empty() { return size_ == 0; }
	uint32_t size() { return size_; }

private:
	LinkedBuffer* head_;
	LinkedBuffer* tail_;
	uint32_t size_;
};

class SendBuffer : noncopyable {
public:
	SendBuffer();
	~SendBuffer();
	void append(const char* buf, uint32_t count);
	void append(Buffer&& buffer);
	void append(Buffer&& buffer, uint32_t nwrote);

	int fill(struct iovec* iov, int iovcnt);

	uint32_t size() { return size_; }
	uint32_t count() { return count_; }

	void retrieve(uint32_t count);

private:
	void append(LinkedBuffer* buffer);

	const static uint32_t kMaxWrite = 64 * 1024;

	LinkedBuffer* head_;
	LinkedBuffer* tail_;
	uint32_t size_;
	uint32_t count_;
};
*/

class Buffer : noncopyable {
public:
	Buffer(uint32_t capacity);
	~Buffer();

	void clear();
	void reset();

private:
	char* buffer_;
	uint32_t capacity_;
	uint32_t begin_;
	uint32_t end_;
	Buffer* next_;

	friend class BufferPool;
	friend class SendBuffer;
	friend class ReceiveBuffer;
}; // end class Buffer

class ListBuffer : noncopyable {
public:
	ListBuffer();

	Buffer* head() { return head_; }
	Buffer* tail() { return tail_; }

	void pushTail(Buffer* buffer);
	Buffer* popHead();

	bool empty() { return size_ == 0; }
	uint32_t size() { return size_; }

private:
	Buffer* head_;
	Buffer* tail_;
	uint32_t size_;
}; // end class ListBuffer

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

	void write(const char* buf, uint32_t count);
	int fill(struct iovec* iov, int iovcnt);
	void retrieve(uint32_t count);

	uint32_t size() { return size_; }
	uint32_t count() { return count_; }
private:
	//ssize_t writeSocket(Socket& socket);
	const static uint32_t kMaxWrite = 64 * 1024;

	BufferPool* pool_;
	uint32_t size_;
	uint32_t count_;
	Buffer* head_;
	Buffer* tail_;

	friend class TcpConnection;
}; // end class SendBuffer
/*
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
*/
} // end namespace catta

#endif // CATTA_NET_BUFFER_H