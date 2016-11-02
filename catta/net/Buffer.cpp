#include <catta/net/Buffer.h>

#include <catta/net/Socket.h>

#include <cstring>

using namespace catta;
Buffer::Buffer(uint32_t capacity)
	: capacity_()
	, read_(0)
	, write_(0)
	, next_(nullptr) {
	buffer_ = static_cast<char*>(std::malloc(capacity));
}

Buffer::~Buffer() {
	if (buffer_) {
		std::free(buffer_);
	}
}

ListBuffer::~ListBuffer() {
	Buffer* buffer = popFront();
	while (buffer) {
		delete buffer;
		buffer = popFront();
	}
}

SendBuffer::SendBuffer(BufferPool* pool)
	: pool_(pool)
	, listBuffer_()
	, size_(0) {
}

SendBuffer::~SendBuffer() {
}

void SendBuffer::write(const void* buf, uint32_t count) {
	if (buf == nullptr || count == 0) {
		return;
	}
	uint32_t writable;
	uint32_t nwrote = 0;
	Buffer* buffer = listBuffer_.back();
	if (buffer) {
		writable = std::min(buffer->writableBytes(), count);
		std::memcpy(buffer->dataWrite(), buf, writable);
		nwrote += writable;
	}
	while (nwrote < count) {
		buffer = pool_->take();
		writable = std::min(buffer->capacity(), count - nwrote);
		std::memcpy(buffer->data(), static_cast<const char*>(buf) + nwrote, writable);
		buffer->hasWritten(writable);

		listBuffer_.pushBack(buffer);
		nwrote += writable;
	}
	size_ += count;
}

int SendBuffer::fill(struct iovec* iov, int iovcnt) {
	Buffer* buffer = listBuffer_.front();
	uint32_t count = 0;
	int i = 0;
	while (buffer && i < iovcnt && count < kMaxWrite) {
		uint32_t readable = buffer->readableBytes();
		iov[i].iov_base = buffer->dataRead();
		iov[i].iov_len = readable;
		buffer = buffer->next();
		++i;
		count += readable;
	}
	return i;
}

void SendBuffer::hasSent(uint32_t count) {
	uint32_t readable;
	uint32_t nread = 0;
	while (nread < count) {
		Buffer* buffer = listBuffer_.front();
		readable = std::min(buffer->readableBytes(), count - nread);
		buffer->hasRead(readable);
		if (buffer->readableBytes() == 0) {
			pool_->put(listBuffer_.popFront());
		}
		nread += readable;
	}
	size_ -= count;
}

/*
ssize_t SendBuffer::writeSocket(Socket& socket) {
	if (!head_->next_) {
		size_t readable = head_->write_ - head_->read_;
		const ssize_t nwrite = socket.write(head_->buffer_ + head_->read_, readable);
		if (nwrite > 0) {
			if (static_cast<size_t>(nwrite) < readable) {
				head_->read_ += nwrite;
			} else {
				head_->read_ = head_->write_ = 0;
			}
			count_ -= nwrite;
		}
		return nwrite;
	}

	if (head_->read_ == 0) {
		size_t readable = head_->write_ - head_->read_;
		const ssize_t nwrite = socket.write(head_->buffer_, readable);
		if (nwrite > 0) {
			if (static_cast<size_t>(nwrite) < readable) {
				head_->read_ += nwrite;
			} else {
				Buffer* buffer = head_;
				head_ = head_->next_;
				buffer->clear();
				pool_->put(buffer);
			}
			count_ -= nwrite;
		}
		return nwrite;
	}

	struct iovec iov[2];
	Buffer* next = head_->next_;
	size_t readable = head_->write_ - head_->read_;
	iov[0].iov_base = head_->buffer_ + head_->read_;
	iov[0].iov_len = readable;
	iov[1].iov_base = next->buffer_;
	iov[1].iov_len = next->write_ - head_->read_;

	const ssize_t nwrite = socket.writev(iov, 2);
	if (nwrite > 0) {
		if (static_cast<size_t>(nwrite) < readable) {
			head_->read_ += nwrite;
		} else {
			head_->read_ = head_->write_ = 0;
			head_->next_ = nullptr;
			pool_->put(head_);
			head_ = next;
			if (static_cast<size_t>(nwrite) > readable) {
				head_->read_ = nwrite - readable;
			}
		}
		count_ -= nwrite;
	}
	return nwrite;
}
*/

/*
ReceiveBuffer::ReceiveBuffer(BufferPool* pool)
	: pool_(pool)
	, count_(0)
	, head_(nullptr)
	, tail_(nullptr) {
}

ReceiveBuffer::~ReceiveBuffer() {
}

void ReceiveBuffer::peek(char* buf, size_t count) {
	
}

void ReceiveBuffer::read(char* buf, size_t count) {
	
}

ssize_t ReceiveBuffer::readSocket(Socket& socket) {
	if (tail_->write_ == 0) {
		const ssize_t nread = socket.read(tail_->buffer_, tail_->capacity_);
		if (nread > 0) {
			tail_->write_ = nread;
			count_ += nread;
		}
		return nread;
	}
	
	if (tail_->write_ == tail_->capacity_) {
		Buffer* next = pool_->next();
		const ssize_t nread = socket.read(next->buffer_, next->capacity_);
		if (nread > 0) {
			tail_->next_ = next;
			tail_ = next;
			tail_->write_ = nread;
			count_ += nread;
		}
		return nread;
	}

	struct iovec iov[2];
	Buffer* next = pool_->next();
	size_t writable = tail_->capacity_ - tail_->write_;
	iov[0].iov_base = tail_->buffer_ + tail_->write_;
	iov[0].iov_len = writable;
	iov[1].iov_base = next->buffer_;
	iov[1].iov_len = next->capacity_;

	const ssize_t nread = socket.readv(iov, 2);
	if (nread > 0) {
		if (static_cast<size_t>(nread) < writable) {
			tail_->write_ += nread;
		} else {
			tail_->write_ = tail_->capacity_;
			if (static_cast<size_t>(nread) > writable)  {
				tail_->next_ = next;
				tail_ = next;
				tail_->write_ = nread - writable;
			}
		}
		count_ += nread;
	}
	return nread;
}*/