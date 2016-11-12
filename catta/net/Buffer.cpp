#include <catta/net/Buffer.h>

#include <cstring>
#include <memory>

using namespace catta;

Buffer::Buffer(const void* buf, uint32_t count)
	: size_(count) {
	buffer_ = static_cast<char*>(std::malloc(count));
}

Buffer::~Buffer() {
	if (buffer_) {
		std::free(buffer_);
	}
}

LinkedBuffer::LinkedBuffer(uint32_t capacity)
	: capacity_(capacity)
	, readIndex_(0)
	, writeIndex_(0)
	, next_(nullptr) {
	buffer_ = static_cast<char*>(std::malloc(capacity_));
}

LinkedBuffer::LinkedBuffer(const void* buf, uint32_t count)
	: capacity_(count)
	, readIndex_(0)
	, writeIndex_(count)
	, next_(nullptr) {
	buffer_ = static_cast<char*>(std::malloc(capacity_));
	std::memcpy(buffer_, buf, count);
}

LinkedBuffer::LinkedBuffer(Buffer&& buffer)
	: buffer_(buffer.buffer_)
	, capacity_(buffer.size_)
	, readIndex_(0)
	, writeIndex_(buffer.size_)
	, next_(nullptr) {
	buffer.buffer_ = nullptr;
	buffer.size_ = 0;
}

LinkedBuffer::LinkedBuffer(Buffer&& buffer, uint32_t offset)
	: buffer_(buffer.buffer_)
	, capacity_(buffer.size_)
	, readIndex_(offset)
	, writeIndex_(buffer.size_)
	, next_(nullptr) {
	buffer.buffer_ = nullptr;
	buffer.size_ = 0;
}

LinkedBuffer::~LinkedBuffer() {
	if (buffer_) {
		std::free(buffer_);
	}
}

ListBuffer::~ListBuffer() {
	for (uint32_t i = 0; i < size_; ++i) {
		LinkedBuffer* buffer = head_;
		head_ = head_->next_;
		delete buffer;
	}
}

void SendBuffer::append(const void* buf, uint32_t count) {
	LinkedBuffer* listBuffer = new LinkedBuffer(buf, count);
	(void)listBuffer;
}

void SendBuffer::append(Buffer&& buffer) {
	LinkedBuffer* listBuffer = new LinkedBuffer(std::move(buffer));
	(void)listBuffer;
}

void SendBuffer::append(Buffer&& buffer, uint32_t offset) {
	LinkedBuffer* listBuffer = new LinkedBuffer(std::move(buffer));
	(void)listBuffer;
}