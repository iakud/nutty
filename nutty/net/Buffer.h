#ifndef NUTTY_NET_BUFFER_H
#define NUTTY_NET_BUFFER_H

#include <cstdint>

struct iovec;

namespace nutty {

class Buffer {
public:
	Buffer(const void* buf, uint32_t count);
	~Buffer();

	inline char* data() { return buffer_; }
	inline uint32_t size() { return size_; }
private:
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	char* buffer_;
	uint32_t size_;

	friend class LinkedBuffer;
}; // end class Buffer

class LinkedBuffer;

class ListBuffer {
public:
	ListBuffer() : head_(nullptr), tail_(nullptr), size_(0) {}

	inline void pushTail(LinkedBuffer* buffer);
	inline void popHead();

	inline LinkedBuffer* head() { return head_; }
	inline LinkedBuffer* tail() { return tail_; }
	inline uint32_t size() { return size_; }
	inline bool empty() { return size_ == 0; }

private:
	ListBuffer(const ListBuffer&) = delete;
	ListBuffer& operator=(const ListBuffer&) = delete;

	LinkedBuffer* head_;
	LinkedBuffer* tail_;
	uint32_t size_;
}; // end class ListBuffer

class SendBuffer {
public:
	SendBuffer();
	~SendBuffer();

	void append(const void* buf, uint32_t count);
	void append(Buffer&& buf);
	void append(Buffer&& buf, uint32_t offset);

	uint32_t size() { return size_; }

private:
	static const uint32_t kSendSize = 8 * 1024;

	SendBuffer(const SendBuffer&) = delete;
	SendBuffer& operator=(const SendBuffer&) = delete;

	inline int buffersSize() { return buffers_.size(); }
	void prepareSend(struct iovec* iov, int& iovcnt);
	void hasSent(uint32_t count);

	ListBuffer buffers_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class SendBuffer

class ReceiveBuffer {
public:
	ReceiveBuffer();
	~ReceiveBuffer();

	void read(void* buf, uint32_t count);
	void peek(void* buf, uint32_t count);

	uint32_t size() { return size_; }

private:
	static const uint32_t kReceiveSize = 8 * 1024;

	ReceiveBuffer(const ReceiveBuffer&) = delete;
	ReceiveBuffer& operator=(const ReceiveBuffer&) = delete;

	void prepareReceive(struct iovec* iov, int& iovcnt);
	void hasReceived(uint32_t count);

	ListBuffer buffers_;
	ListBuffer extendBuffers_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class ReceiveBuffer

} // end namespace nutty

#endif // NUTTY_NET_BUFFER_H