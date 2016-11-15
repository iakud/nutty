#include <catta/net/Buffer.h>

#include <cstring>
#include <memory>

#include <arpa/inet.h>

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

SendBuffer::SendBuffer()
	: size_(0)
	, iovsize_(kIovSizeInit)
	, iovcnt_(0) {
	iov_ = static_cast<struct iovec*>(std::malloc(sizeof(struct iovec) * kIovSizeInit));
}

SendBuffer::~SendBuffer() {
	if (iov_) {
		std::free(iov_);
	}
}

void SendBuffer::append(const void* buf, uint32_t count) {
	LinkedBuffer* linkedBuffer = new LinkedBuffer(buf, count);
	listBuffer_.pushBack(linkedBuffer);
}

void SendBuffer::append(Buffer&& buffer) {
	LinkedBuffer* linkedBuffer = new LinkedBuffer(std::move(buffer));
	listBuffer_.pushBack(linkedBuffer);
}

void SendBuffer::append(Buffer&& buffer, uint32_t offset) {
	LinkedBuffer* linkedBuffer = new LinkedBuffer(std::move(buffer), offset);
	listBuffer_.pushBack(linkedBuffer);
}

void SendBuffer::prepareSend() {
	if (iovsize_ < static_cast<int>(listBuffer_.size()) && iovsize_ < kIovSizeMax) {
		resize(iovsize_ * 2);
	}
	iovcnt_ = 0;
	uint32_t nwrote = 0;
	LinkedBuffer* linkedBuffer = listBuffer_.front();
	while (linkedBuffer && iovcnt_ < iovsize_ && nwrote < kMaxSend) {
		uint32_t readableSize = linkedBuffer->readableSize();
		iov_[iovcnt_].iov_base = linkedBuffer->dataRead();
		iov_[iovcnt_].iov_len = readableSize;
		++iovcnt_;
		nwrote += readableSize;
		linkedBuffer = linkedBuffer->next();
	}
}

void SendBuffer::resize(int iovsize) {
	if (iov_) {
		std::free(iov_);
	}
	iov_ = static_cast<struct iovec*>(std::malloc(sizeof(struct iovec) * iovsize));
	iovsize_ = iovsize;
}