#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

#include <catta/base/noncopyable.h>

#include <cstdint>
#include <memory>
#include <unistd.h>

struct iovec;
class Socket;

namespace catta {

class Buffer : noncopyable {
public:
	Buffer(uint32_t capacity);
	~Buffer();

	inline char* data() { return buffer_; }
	inline char* dataRead() { return buffer_ + read_; }
	inline char* dataWrite() { return buffer_ + write_; }

	inline uint32_t capacity() { return capacity_; }
	inline uint32_t writableBytes() { return capacity_ - write_; }
	inline uint32_t readableBytes() { return write_ - read_; }

	inline Buffer* next() { return next_; }

	inline void hasRead(uint32_t count) { read_ += count; }
	inline void hasWritten(uint32_t count) { write_ += count; }
	inline void clear() { read_ = write_ = 0; }

private:
	char* buffer_;
	uint32_t capacity_;
	uint32_t read_;
	uint32_t write_;
	Buffer* next_;

	friend class ListBuffer;
}; // end class Buffer

class ListBuffer : noncopyable {
public:
	ListBuffer() : head_(nullptr), tail_(nullptr), size_(0) {}
	~ListBuffer();

	inline void pushBack(Buffer* buffer) {
		if (buffer) {
			buffer->next_ = nullptr;
			if (size_ == 0) {
				head_ = tail_ = buffer;
			} else {
				tail_->next_ = buffer;
				tail_ = buffer;
			}
			++size_;
		}
	}

	inline Buffer* popFront() {
		Buffer* buffer = head_;
		if (buffer) {
			if (buffer->next_) {
				head_ = buffer->next_;
				buffer->next_ = nullptr;
			} else {
				head_ = tail_ = nullptr;
			}
			--size_;
		}
		return buffer;
	}

	inline Buffer* front() { return head_; }
	inline Buffer* back() { return tail_; }
	inline uint32_t size() { return size_; }
	inline bool empty() { return size() == 0; }

private:
	Buffer* head_;
	Buffer* tail_;
	uint32_t size_;
}; // end class ListBuffer

class BufferPool : noncopyable {
public:
	BufferPool(uint32_t capacity) : capacity_(capacity) {}

	inline void put(Buffer* buffer) {
		if (buffer) {
			if (listBuffer_.size() < capacity_) {
				listBuffer_.pushBack(buffer);
			} else {
				delete buffer;
			}
		}
	}

	inline Buffer* take() {
		Buffer* buffer;
		if (listBuffer_.empty()) {
			buffer = new Buffer(1024 * 8);
		} else {
			buffer = listBuffer_.popFront();
			buffer->clear();
		}
		return buffer;
	}

private:
	const uint32_t capacity_;
	ListBuffer listBuffer_;
}; // end class BufferPool

class SendBuffer : noncopyable {
public:
	SendBuffer(BufferPool* pool);
	~SendBuffer();

	void write(const void* buf, uint32_t count);
	int fill(struct iovec* iov, int iovcnt);
	void hasSent(uint32_t count);

	uint32_t count() { return listBuffer_.size(); }
	uint32_t size() { return size_; }

private:
	const static uint32_t kMaxWrite = 64 * 1024;

	BufferPool* pool_;
	ListBuffer listBuffer_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class SendBuffer
/*
class ReceiveBuffer : noncopyable {
public:
	ReceiveBuffer(BufferPool* pool);
	~ReceiveBuffer();

	void peek(void* buf, uint32_t count);
	void read(void* buf, uint32_t count);
	void retrieve(uint32_t count);

	uint32_t size() { return listBuffer_.size(); }
	uint32_t count() { return count_; }

private:
	BufferPool* pool_;
	ListBuffer listBuffer_;
	size_t count_;

	friend class TcpConnection;
}; // end class ReceiveBuffer
*/
} // end namespace catta

#endif // CATTA_NET_BUFFER_H