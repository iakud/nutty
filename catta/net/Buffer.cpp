#include <catta/net/Buffer.h>

#include <catta/net/Socket.h>

#include <cstring>

using namespace catta;

const size_t Buffer::kCapacity;

Buffer::Buffer()
	: capacity_(kCapacity)
	, write_(0)
	, read_(0)
	, next_(nullptr) {

}

Buffer::~Buffer() {

}

void Buffer::clear() {
	read_ = write_ = 0;
}

void Buffer::reset() {
	read_ = write_ = 0;
	next_ = nullptr;	
}

BufferPool::BufferPool(uint32_t capacity)
	: capacity_(capacity)
	, count_(0) {
}

BufferPool::~BufferPool() {

}

void BufferPool::put(Buffer* buffer) {
	if (buffer == nullptr) {
		return;
	}
	if (count_ < capacity_) {
		if (count_ == 0) {
			head_ = tail_ = buffer;
		} else {
			tail_->next_ = buffer;
			tail_ = buffer;
		}
		++count_;
	} else {
		delete buffer;
	}
}

Buffer* BufferPool::take() {
	if (count_ > 0) {
		Buffer* buffer = head_;
		if (count_ > 1) {
			head_ = head_->next_;
			buffer->next_ = nullptr;
		} else {
			head_ = tail_ = nullptr;
		}
		--count_;
		return buffer;
	} else {
		return new Buffer();
	}
}

SendBuffer::SendBuffer(BufferPool* pool)
	: pool_(pool)
	, count_(0)
	, head_(nullptr)
	, tail_(nullptr) {
}

SendBuffer::~SendBuffer() {
}

void SendBuffer::write(const char* buf, size_t count) {
	if (buf == nullptr || count == 0) {
		return;
	}
	size_t writable = tail_->capacity_ - tail_->write_;
	size_t writecnt = count < writable ? count : writable;
	std::memcpy(tail_->buffer_ + tail_->write_, buf, writecnt);
	size_t nwrite = writecnt;
	while (nwrite < count) {
		tail_->next_ = pool_->take();
		tail_ = tail_->next_;
		size_t remaincnt = count - nwrite;
		writecnt = remaincnt < tail_->capacity_ ? remaincnt : tail_->capacity_;
		std::memcpy(tail_->buffer_, buf + nwrite, writecnt);
		tail_->write_ = writecnt;
		nwrite += writecnt;
	}
	count_ += count;
}

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
}