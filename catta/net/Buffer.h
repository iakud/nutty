#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

#include <catta/base/noncopyable.h>

#include <cstdint>

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

	inline void pushBack(LinkedBuffer* buffer) {
		if (buffer == nullptr || buffer->next_) {
			return;
		}
		if (tail_) {
			tail_->next_ = buffer;
			tail_ = buffer;
		} else {
			head_ = tail_ = buffer;
		}
		++size_;
	}

	inline LinkedBuffer* popFront() {
		if (head_ == nullptr) {
			return nullptr;
		}
		LinkedBuffer* buffer = head_;
		if (head_->next_) {
			head_ = head_->next_;
			buffer->next_ = nullptr;
		} else {
			head_ = tail_ = nullptr;
		}
		--size_;
		return buffer;
	}

	inline LinkedBuffer* front() { return head_; }
	inline LinkedBuffer* back() { return tail_; }
	inline uint32_t size() { return size_; }
	inline bool empty() { return size_ == 0; }

private:
	LinkedBuffer* head_;
	LinkedBuffer* tail_;
	uint32_t size_;
}; // end class ListBuffer

class SendBuffer : noncopyable {
public:
	void append(const void* buf, uint32_t count);
	void append(Buffer&& buffer);
	void append(Buffer&& buffer, uint32_t offset);
private:
	ListBuffer listBuffer_;
}; // end class SendBuffer

class ReceiveBuffer : noncopyable {
public:

private:
	ListBuffer listBuffer_;
}; // end class ReceiveBuffer

} // end namespace catta

#endif // CATTA_NET_BUFFER_H