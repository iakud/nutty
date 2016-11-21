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
	iov_ = static_cast<struct iovec*>(std::malloc(sizeof(struct iovec) * iovsize_));
}

SendBuffer::~SendBuffer() {
	if (iov_) {
		std::free(iov_);
	}
}

void SendBuffer::append(const void* buf, uint32_t count) {
	LinkedBuffer* buffer = new LinkedBuffer(buf, count);
	buffers_.pushBack(buffer);
	size_ += count;
}

void SendBuffer::append(Buffer&& buf) {
	LinkedBuffer* buffer = new LinkedBuffer(std::move(buf));
	buffers_.pushBack(buffer);
	size_ += buf.size();
}

void SendBuffer::append(Buffer&& buf, uint32_t offset) {
	LinkedBuffer* buffer = new LinkedBuffer(std::move(buf), offset);
	buffers_.pushBack(buffer);
	size_ += buf.size() - offset;
}

void SendBuffer::prepareSend() {
	if (iovsize_ < static_cast<int>(buffers_.size()) && iovsize_ < kIovSizeMax) {
		resize(iovsize_ * 2);
	}
	iovcnt_ = 0;
	uint32_t nwrote = 0;
	LinkedBuffer* buffer = buffers_.front();
	while (buffer && iovcnt_ < iovsize_ && nwrote < kMaxSend) {
		uint32_t readableSize = buffer->readableSize();
		iov_[iovcnt_].iov_base = buffer->dataRead();
		iov_[iovcnt_].iov_len = readableSize;
		++iovcnt_;
		nwrote += readableSize;
		buffer = buffer->next();
	}
}

void SendBuffer::hasSent(uint32_t count) {
	uint32_t nwrote = 0;
	while (!buffers_.empty() && nwrote < count) {
		LinkedBuffer* buffer = buffers_.front();
		uint32_t readableSize = std::min(buffer->readableSize(), count - nwrote);
		buffer->hasRead(readableSize);
		if (buffer->empty()) {
			buffers_.popFront();
			delete buffer;
		}
		nwrote += readableSize;
	}
	size_ -= nwrote;
}

void SendBuffer::resize(int iovsize) {
	if (iov_) {
		std::free(iov_);
	}
	iov_ = static_cast<struct iovec*>(std::malloc(sizeof(struct iovec) * iovsize));
	iovsize_ = iovsize;
}

ReceiveBuffer::ReceiveBuffer()
	: size_(0)
	, iovsize_(kMaxReceive/kReceiveSize + 1)
	, iovcnt_(0) {
	iov_ = static_cast<struct iovec*>(std::malloc(sizeof(struct iovec) * iovsize_));
}

ReceiveBuffer::~ReceiveBuffer() {
	if (iov_) {
		std::free(iov_);
	}
}

void ReceiveBuffer::prepareReceive() {
	iovcnt_ = 0;
	uint32_t nwrote = 0;
	LinkedBuffer* buffer = buffers_.back();
	uint32_t writableSize = buffer->writableSize();
	if (writableSize > 0) {
		iov_[iovcnt_].iov_base = buffer->dataWrite();
		iov_[iovcnt_].iov_len = writableSize;
		++iovcnt_;
		nwrote += writableSize;
	}
	buffer = extendBuffers_.front();
	while (iovcnt_ < iovsize_ && nwrote < kMaxReceive) {
		if (!buffer) {
			buffer = new LinkedBuffer(kReceiveSize);
			extendBuffers_.pushBack(buffer);
		}
		writableSize = buffer->capacity();
		iov_[iovcnt_].iov_base = buffer->data();
		iov_[iovcnt_].iov_len = writableSize;
		++iovcnt_;
		nwrote += writableSize;
		buffer = buffer->next();
	}
}

void ReceiveBuffer::hasReceived(uint32_t count) {
	uint32_t nwrote = 0;
	LinkedBuffer* buffer = buffers_.back();
	uint32_t writableSize = std::min(buffer->writableSize(), count);
	if (writableSize > 0) {
		buffer->hasWritten(writableSize);
		nwrote += writableSize;
	}
	buffer = extendBuffers_.front();
	while (buffer && nwrote < count) {
		writableSize = std::min(buffer->writableSize(), count - nwrote);
		buffer->hasWritten(writableSize);
		nwrote += writableSize;
		extendBuffers_.popFront();
		buffers_.pushBack(buffer);
		buffer = extendBuffers_.front();
	}
	size_ += nwrote;
}