#include <catta/net/Buffer.h>

#include <cstring>
#include <memory>

#include <arpa/inet.h>

namespace catta {

class LinkedBuffer : noncopyable {
public:
	LinkedBuffer(uint32_t capacity)
		: capacity_(capacity)
		, readIndex_(0)
		, writeIndex_(0)
		, next_(nullptr) {
		buffer_ = static_cast<char*>(std::malloc(capacity_));
	}

	LinkedBuffer(const void* buf, uint32_t count)
		: capacity_(count)
		, readIndex_(0)
		, writeIndex_(count)
		, next_(nullptr) {
		buffer_ = static_cast<char*>(std::malloc(capacity_));
		std::memcpy(buffer_, buf, count);
	}

	LinkedBuffer(Buffer&& buffer)
		: buffer_(buffer.buffer_)
		, capacity_(buffer.size_)
		, readIndex_(0)
		, writeIndex_(buffer.size_)
		, next_(nullptr) {
		buffer.buffer_ = nullptr;
		buffer.size_ = 0;
	}

	LinkedBuffer(Buffer&& buffer, uint32_t offset)
		: buffer_(buffer.buffer_)
		, capacity_(buffer.size_)
		, readIndex_(offset)
		, writeIndex_(buffer.size_)
		, next_(nullptr) {
		buffer.buffer_ = nullptr;
		buffer.size_ = 0;
	}

	~LinkedBuffer() {
		if (buffer_) {
			std::free(buffer_);
		}
	}

	inline char* data() { return buffer_; }
	inline char* dataRead() { return buffer_ + readIndex_; }
	inline char* dataWrite() { return buffer_ + writeIndex_; }

	inline uint32_t capacity() { return capacity_; }
	inline uint32_t readableSize() { return writeIndex_ - readIndex_; }
	inline uint32_t writableSize() { return capacity_ - writeIndex_; }

	inline void hasWritten(uint32_t count) { writeIndex_ += count; }
	inline void hasRead(uint32_t count) { readIndex_ += count; }
	inline void reset() { readIndex_ = writeIndex_ = 0; }
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

} // end namespace catta

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

void ListBuffer::pushTail(LinkedBuffer* buffer) {
	if (buffer == nullptr || buffer->next_) return;
	if (tail_) {
		tail_->next_ = buffer;
		tail_ = buffer;
	} else {
		head_ = tail_ = buffer;
	}
	++size_;
}

void ListBuffer::popHead() {
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

SendBuffer::SendBuffer()
	: size_(0) {
}

SendBuffer::~SendBuffer() {
	while (!buffers_.empty()) {
		LinkedBuffer* buffer = buffers_.head();
		buffers_.popHead();
		delete buffer;
	}
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
	uint32_t nread = 0;
	LinkedBuffer* buffer = buffers_.head();
	while (buffer && nread < kSendSize) {
		uint32_t readableSize = buffer->readableSize();
		iov[iovcnt].iov_base = buffer->dataRead();
		iov[iovcnt].iov_len = readableSize;
		++iovcnt;
		nread += readableSize;
		buffer = buffer->next();
	}
}

void SendBuffer::hasSent(uint32_t count) {
	uint32_t nread = 0;
	while (nread < count && buffers_.head()) {
		LinkedBuffer* buffer = buffers_.head();
		uint32_t readableSize = std::min(buffer->readableSize(), count - nread);
		buffer->hasRead(readableSize);
		nread += readableSize;
		if (buffer->empty()) {
			buffers_.popHead();
			delete buffer;
		}
	}
	size_ -= nread;
}

ReceiveBuffer::ReceiveBuffer()
	: size_(0) {
}

ReceiveBuffer::~ReceiveBuffer() {
	while (!buffers_.empty()) {
		LinkedBuffer* buffer = buffers_.head();
		buffers_.popHead();
		delete buffer;
	}
	while (!extendBuffers_.empty()) {
		LinkedBuffer* buffer = extendBuffers_.head();
		extendBuffers_.popHead();
		delete buffer;
	}
}

void ReceiveBuffer::read(void* buf, uint32_t count) {
	uint32_t nread = 0;
	while (nread < count && buffers_.head()) {
		LinkedBuffer* buffer = buffers_.head();
		uint32_t readableSize = std::min(buffer->readableSize(), count - nread);
		std::memcpy(static_cast<char*>(buf) + nread, buffer->dataRead(), readableSize);
		buffer->hasRead(readableSize);
		nread += readableSize;
		if (buffer->empty()) {
			buffers_.popHead();
			buffer->reset();
			extendBuffers_.pushTail(buffer);
		}
	}
	size_ -= nread;
}

void ReceiveBuffer::peek(void* buf, uint32_t count) {
	uint32_t nread = 0;
	LinkedBuffer* buffer = buffers_.head();
	while (nread < count && buffer) {
		uint32_t readableSize = std::min(buffer->readableSize(), count - nread);
		std::memcpy(static_cast<char*>(buf) + nread, buffer->dataRead(), readableSize);
		nread += readableSize;
		if (readableSize == buffer->readableSize()) {
			buffer = buffer->next();
		}
	}
}

void ReceiveBuffer::prepareReceive(struct iovec* iov, int& iovcnt) {
	iovcnt = 0;
	uint32_t nwrote = 0;
	
	if (buffers_.tail()) {
		LinkedBuffer* buffer = buffers_.tail();
		uint32_t writableSize = buffer->writableSize();
		if (writableSize > 0) {
			iov[iovcnt].iov_base = buffer->dataWrite();
			iov[iovcnt].iov_len = writableSize;
			++iovcnt;
			nwrote += writableSize;
		}
	}
	if (nwrote < kReceiveSize) {
		LinkedBuffer* buffer = extendBuffers_.head();
		if (!buffer) {
			buffer = new LinkedBuffer(kReceiveSize);
			extendBuffers_.pushTail(buffer);
		}
		uint32_t writableSize = buffer->capacity();
		iov[iovcnt].iov_base = buffer->data();
		iov[iovcnt].iov_len = writableSize;
		++iovcnt;
		nwrote += writableSize;
		// buffer = buffer->next();
	}
}

void ReceiveBuffer::hasReceived(uint32_t count) {
	uint32_t nwrote = 0;
	if (buffers_.tail()) {
		LinkedBuffer* buffer = buffers_.tail();
		uint32_t writableSize = std::min(buffer->writableSize(), count);
		if (writableSize > 0) {
			buffer->hasWritten(writableSize);
			nwrote += writableSize;
		}
	}
	if (nwrote < count && extendBuffers_.head()) {
		LinkedBuffer* buffer = extendBuffers_.head();
		uint32_t writableSize = std::min(buffer->writableSize(), count - nwrote);
		buffer->hasWritten(writableSize);
		nwrote += writableSize;
		extendBuffers_.popHead();
		buffers_.pushTail(buffer);
	}
	size_ += nwrote;
}