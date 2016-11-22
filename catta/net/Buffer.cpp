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
	: size_(0) {
}

SendBuffer::~SendBuffer() {
}

void SendBuffer::append(const void* buf, uint32_t count) {
	LinkedBuffer* buffer = new LinkedBuffer(buf, count);
	buffers_.pushTail(buffer);
	size_ += count;
}

void SendBuffer::append(Buffer&& buf) {
	LinkedBuffer* buffer = new LinkedBuffer(std::move(buf));
	buffers_.pushTail(buffer);
	size_ += buf.size();
}

void SendBuffer::append(Buffer&& buf, uint32_t offset) {
	LinkedBuffer* buffer = new LinkedBuffer(std::move(buf), offset);
	buffers_.pushTail(buffer);
	size_ += buf.size() - offset;
}

void SendBuffer::prepareSend(struct iovec* iov, int& iovcnt) {
	iovcnt = 0;
	uint32_t nwrote = 0;
	LinkedBuffer* buffer = buffers_.head();
	while (buffer && nwrote < kSendSize) {
		uint32_t readableSize = buffer->readableSize();
		iov[iovcnt].iov_base = buffer->dataRead();
		iov[iovcnt].iov_len = readableSize;
		++iovcnt;
		nwrote += readableSize;
		buffer = buffer->next();
	}
}

void SendBuffer::hasSent(uint32_t count) {
	uint32_t nwrote = 0;
	while (nwrote < count && buffers_.head()) {
		LinkedBuffer* buffer = buffers_.head();
		uint32_t readableSize = std::min(buffer->readableSize(), count - nwrote);
		buffer->hasRead(readableSize);
		if (buffer->empty()) {
			buffers_.popHead();
			delete buffer;
		}
		nwrote += readableSize;
	}
	size_ -= nwrote;
}

ReceiveBuffer::ReceiveBuffer()
	: size_(0) {
}

ReceiveBuffer::~ReceiveBuffer() {
}

void ReceiveBuffer::prepareReceive(struct iovec* iov, int& iovcnt) {
	iovcnt = 0;
	uint32_t nwrote = 0;
	LinkedBuffer* buffer = buffers_.tail();
	uint32_t writableSize = buffer->writableSize();
	if (writableSize > 0) {
		iov[iovcnt].iov_base = buffer->dataWrite();
		iov[iovcnt].iov_len = writableSize;
		++iovcnt;
		nwrote += writableSize;
	}
	if (nwrote < kReceiveSize) {
		buffer = extendBuffers_.head();
		if (!buffer) {
			buffer = new LinkedBuffer(kReceiveSize);
			extendBuffers_.pushTail(buffer);
		}
		writableSize = buffer->capacity();
		iov[iovcnt].iov_base = buffer->data();
		iov[iovcnt].iov_len = writableSize;
		++iovcnt;
		nwrote += writableSize;
		buffer = buffer->next();
	}
}

void ReceiveBuffer::hasReceived(uint32_t count) {
	uint32_t nwrote = 0;
	LinkedBuffer* buffer = buffers_.tail();
	uint32_t writableSize = std::min(buffer->writableSize(), count);
	if (writableSize > 0) {
		buffer->hasWritten(writableSize);
		nwrote += writableSize;
	}
	if (nwrote < count && extendBuffers_.head()) {
		buffer = extendBuffers_.head();
		writableSize = std::min(buffer->writableSize(), count - nwrote);
		buffer->hasWritten(writableSize);
		nwrote += writableSize;
		extendBuffers_.popHead();
		buffers_.pushTail(buffer);
	}
	size_ += nwrote;
}