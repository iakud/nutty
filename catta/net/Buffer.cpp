#include <catta/net/Buffer.h>

#include <cstring>
#include <memory>

using namespace catta;

Buffer::Buffer(uint32_t capacity)
	: capacity_(capacity)
	, readIndex_(0)
	, writeIndex_(0)
	, next_(nullptr) {
	buffer_ = static_cast<char*>(std::malloc(capacity_));
}

Buffer::Buffer(const void* buf, uint32_t count)
	: capacity_(count)
	, readIndex_(0)
	, writeIndex_(count)
	, next_(nullptr) {
	buffer_ = static_cast<char*>(std::malloc(capacity_));
	std::memcpy(buffer_, buf, count);
}

Buffer::Buffer(Buffer&& other)
	: buffer_(other.buffer_)
	, capacity_(other.capacity_)
	, readIndex_(other.readIndex_)
	, writeIndex_(other.writeIndex_)
	, next_(other.next_) {
	other.buffer_ = nullptr;
	other.capacity_ = 0;
	other.readIndex_ = 0;
	other.writeIndex_ = 0;
	other.next_ = nullptr;
}

Buffer::~Buffer() {
	if (buffer_) {
		std::free(buffer_);
	}
}

void SendBuffer::append(Buffer&& buffer) {
	
}