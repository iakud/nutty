#include <nutty/net/SendBuffer.h>

#include <nutty/net/ReceiveBuffer.h>
#include <nutty/net/Buffer.h>

#include <arpa/inet.h>

using namespace nutty;

SendBuffer::SendBuffer()
	: size_(0) {
}

SendBuffer::~SendBuffer() {
	for (Buffer*& buffer : buffers_) {
		delete buffer;
	}
}

void SendBuffer::append(const void* buf, size_t count) {
	Buffer* buffer = new Buffer(buf, count);
	buffers_.push_back(buffer);
	size_ += count;
}

void SendBuffer::append(Buffer&& buf) {
	Buffer* buffer = new Buffer(std::move(buf));
	buffers_.push_back(buffer);
	size_ += buffer->readableSize();
}

void SendBuffer::append(const ReceiveBuffer& receiveBuffer) {
	Buffer* buffer = new Buffer(receiveBuffer.size());
	receiveBuffer.peek(buffer->data(), buffer->capacity());
	buffers_.push_back(buffer);
	size_ += buffer->capacity();
}

void SendBuffer::append(const ReceiveBuffer& receiveBuffer, size_t offset) {
	Buffer* buffer = new Buffer(receiveBuffer.size() - offset);
	receiveBuffer.peek(buffer->data(), offset, buffer->capacity());
	buffers_.push_back(buffer);
	size_ += buffer->capacity();
}

void SendBuffer::prepareSend(struct iovec* iov, int& iovcnt) const {
	iovcnt = 0;
	size_t nread = 0;
	for (Buffer* buffer : buffers_) {
		if (nread >= kSendSize) break;
		size_t readableSize = buffer->readableSize();
		iov[iovcnt].iov_base = buffer->dataRead();
		iov[iovcnt].iov_len = readableSize;
		++iovcnt;
		nread += readableSize;
	}
}

void SendBuffer::hasSent(size_t count) {
	size_t nread = 0;
	while (nread < count && !buffers_.empty()) {
		Buffer* buffer = buffers_.front();
		size_t readableSize = std::min(buffer->readableSize(), count - nread);
		buffer->hasRead(readableSize);
		nread += readableSize;
		if (buffer->empty()) {
			buffers_.pop_front();
			delete buffer;
		}
	}
	size_ -= nread;
}
