#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

#include <catta/base/noncopyable.h>

#include <cstdint>

struct iovec;

namespace catta {

class Buffer : noncopyable {
public:
	Buffer(const void* buf, uint32_t count);
	~Buffer();

	inline char* data() { return buffer_; }
	inline uint32_t size() { return size_; }
private:
	char* buffer_;
	uint32_t size_;

	friend class LinkedBuffer;
}; // end class Buffer

class LinkedBuffer : noncopyable {
public:
	LinkedBuffer(uint32_t capacity);
	LinkedBuffer(const void* buf, uint32_t count);
	LinkedBuffer(Buffer&& buffer);
	LinkedBuffer(Buffer&& buffer, uint32_t offset);
	~LinkedBuffer();

	inline char* data() { return buffer_; }
	inline char* dataRead() { return buffer_ + readIndex_; }
	inline char* dataWrite() { return buffer_ + writeIndex_; }

	inline uint32_t capacity() { return capacity_; }
	inline uint32_t readableSize() { return writeIndex_ - readIndex_; }
	inline uint32_t writableSize() { return capacity_ - writeIndex_; }

	inline void hasWritten(uint32_t count) { writeIndex_ += count; }
	inline void hasRead(uint32_t count) { readIndex_ += count; }
	inline bool empty() { return readIndex_ == writeIndex_; }
	inline bool full() { return writeIndex_ == capacity_; }

	inline LinkedBuffer* next() { return next_; }

private:
	char* buffer_;
	uint32_t capacity_;
	uint32_t readIndex_;
	uint32_t writeIndex_;
	LinkedBuffer* next_;

	friend class ListBuffer;
}; // end class LinkedBuffer

class ListBuffer : noncopyable {
public:
	ListBuffer() : head_(nullptr), tail_(nullptr), size_(0) {}
	~ListBuffer();

	inline void pushTail(LinkedBuffer* buffer) {
		if (buffer == nullptr || buffer->next_) return;
		if (tail_) {
			tail_->next_ = buffer;
			tail_ = buffer;
		} else {
			head_ = tail_ = buffer;
		}
		++size_;
	}

	inline void popHead() {
		if (head_ == nullptr) return;
		LinkedBuffer* buffer = head_;
		if (head_->next_) {
			head_ = head_->next_;
			buffer->next_ = nullptr;
		} else {
			head_ = tail_ = nullptr;
		}
		--size_;
	}

	inline LinkedBuffer* head() { return head_; }
	inline LinkedBuffer* tail() { return tail_; }
	inline uint32_t size() { return size_; }
	inline bool empty() { return size_ == 0; }

private:
	LinkedBuffer* head_;
	LinkedBuffer* tail_;
	uint32_t size_;
}; // end class ListBuffer

class SendBuffer : noncopyable {
public:
	SendBuffer();
	~SendBuffer();

	void append(const void* buf, uint32_t count);
	void append(Buffer&& buf);
	void append(Buffer&& buf, uint32_t offset);

	uint32_t size() { return size_; }

private:
	inline int buffersSize() { return buffers_.size(); }
	void prepareSend(struct iovec* iov, int& iovcnt);
	void hasSent(uint32_t count);

private:
	static const uint32_t kSendSize = 8 * 1024;

	ListBuffer buffers_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class SendBuffer

class ReceiveBuffer : noncopyable {
public:
	ReceiveBuffer();
	~ReceiveBuffer();

	uint32_t size() { return size_; }

private:
	void prepareReceive(struct iovec* iov, int& iovcnt);
	void hasReceived(uint32_t count);

private:
	static const uint32_t kReceiveSize = 8 * 1024;

	ListBuffer buffers_;
	ListBuffer extendBuffers_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class ReceiveBuffer

} // end namespace catta

#endif // CATTA_NET_BUFFER_H