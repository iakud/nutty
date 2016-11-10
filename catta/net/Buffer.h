#ifndef CATTA_NET_BUFFER_H
#define CATTA_NET_BUFFER_H

#include <catta/base/noncopyable.h>

#include <cstdint>

namespace catta {

class Buffer : noncopyable {
public:
	Buffer(uint32_t capacity);
	Buffer(const void* buf, uint32_t count);
	Buffer(Buffer&& other);
	~Buffer();

	inline char* data() { return buffer_; }
	inline char* dataRead() { return buffer_ + readIndex_; }
	inline char* dataWrite() { return buffer_ + writeIndex_; }

	inline uint32_t capacity() { return capacity_; }
	inline uint32_t readableSize() { return writeIndex_ - readIndex_; }
	inline uint32_t writableSize() { return capacity_ - writeIndex_; }

	Buffer* next() { return next_; }

private:
	char* buffer_;
	uint32_t capacity_;
	uint32_t readIndex_;
	uint32_t writeIndex_;
	Buffer* next_;
}; // end class Buffer

class ListBuffer : noncopyable {
public:

private:
	Buffer* head_;
	Buffer* tail_;
	uint32_t size_;
}; // end class ListBuffer

class SendBuffer : noncopyable {
public:
	void append(Buffer&& buffer);
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