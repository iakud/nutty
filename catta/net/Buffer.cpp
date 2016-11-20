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
	LinkedBuffer* linkedBuffer = new LinkedBuffer(buf, count);
	listBuffer_.pushBack(linkedBuffer);
	size_ += linkedBuffer->readableSize();
}

void SendBuffer::append(Buffer&& buffer) {
	LinkedBuffer* linkedBuffer = new LinkedBuffer(std::move(buffer));
	listBuffer_.pushBack(linkedBuffer);
	size_ += linkedBuffer->readableSize();
}

void SendBuffer::append(Buffer&& buffer, uint32_t offset) {
	LinkedBuffer* linkedBuffer = new LinkedBuffer(std::move(buffer), offset);
	listBuffer_.pushBack(linkedBuffer);
	size_ += linkedBuffer->readableSize();
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

void SendBuffer::hasSent(uint32_t count) {
	uint32_t nwrote = 0;
	while (!listBuffer_.empty() && nwrote < count) {
		LinkedBuffer* linkedBuffer = listBuffer_.front();
		uint32_t readableSize = std::min(linkedBuffer->readableSize(), count - nwrote);
		linkedBuffer->hasRead(readableSize);
		if (linkedBuffer->empty()) {
			listBuffer_.popFront();
			delete linkedBuffer;
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
	LinkedBuffer* linkedBuffer = new LinkedBuffer(kReceiveSize);
	listBuffer_.pushBack(linkedBuffer);
	buffer_ = linkedBuffer;
}

ReceiveBuffer::~ReceiveBuffer() {
	if (iov_) {
		std::free(iov_);
	}
}
//FIXME :
void ReceiveBuffer::prepareReceive() {
	iovcnt_ = 0;
	uint32_t nwrote = 0;
	LinkedBuffer* linkedBuffer = buffer_;
	while (iovcnt_ < iovsize_ && nwrote < kMaxReceive) {
		if (!linkedBuffer) {
			linkedBuffer = new LinkedBuffer(kReceiveSize);
			listBuffer_.pushBack(linkedBuffer);
		}
		uint32_t writableSize = linkedBuffer->writableSize();
		if (writableSize > 0) {
			iov_[iovcnt_].iov_base = linkedBuffer->dataWrite();
			iov_[iovcnt_].iov_len = writableSize;
			++iovcnt_;
			nwrote += writableSize;
		}
		linkedBuffer = linkedBuffer->next();
	}
}

void ReceiveBuffer::hasReceived(uint32_t count) {
	uint32_t nwrote = 0;
	while (buffer_ && nwrote < count) {
		uint32_t writableSize = std::min(buffer_->writableSize(), count - nwrote);
		if (writableSize > 0) {
			buffer_->hasWritten(writableSize);
			nwrote += writableSize;
		}
		if (buffer_->full()) {
			buffer_ = buffer_->next();
		}
	}
	size_ += nwrote;
}