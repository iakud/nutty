#include <catta/net/Buffer.h>

#include <catta/net/Socket.h>

#include <algorithm>
#include <cstring>
#include <cstdlib>

using namespace catta;
/*
Buffer::Buffer(const char* buf, uint32_t count) {
	if (buf == nullptr || count == 0) {
		buffer_ = nullptr;
		count_ = 0;
	} else {
		buffer_ = static_cast<char*>(std::malloc(count));
		std::memcpy(buffer_, buf, count);
		count_ = count;
	}
}

Buffer::~Buffer() {
	if (buffer_) {
		std::free(buffer_);
	}
}

Buffer::Buffer(Buffer&& buffer) {

}

LinkedBuffer::LinkedBuffer(const char* buf, uint32_t count)
	: read_(0)
	, next_(nullptr) {
	if (buf == nullptr || count == 0) {
		buffer_ = nullptr;
		capacity_ = 0;
		next_ = nullptr;
	} else {
		buffer_ = new char[count];
		std::memcpy(buffer_, buf, count);
		capacity_ = count;
		write_ = count;
	}
}

LinkedBuffer::LinkedBuffer(Buffer&& buffer)
	: buffer_(buffer.buffer_)
	, capacity_(buffer.count_)
	, write_(buffer.count_)
	, read_(0)
	, next_(nullptr) {
	buffer.buffer_ = nullptr;
	buffer.count_ = 0;
}

LinkedBuffer::~LinkedBuffer() {
	if (buffer_) {
		std::free(buffer_);
	}
}

SendBuffer::SendBuffer() {
	
}

SendBuffer::~SendBuffer() {

}

void SendBuffer::append(const char* buf, uint32_t count) {
	LinkedBuffer* linkedBuffer = new LinkedBuffer(buf, count);
	append(linkedBuffer);
}

void SendBuffer::append(Buffer&& buffer) {
	LinkedBuffer* linkedBuffer = new LinkedBuffer(std::move(buffer));
	append(linkedBuffer);
}

void SendBuffer::append(Buffer&& buffer, uint32_t nwrote) {
	if (nwrote < buffer.size()) {
		LinkedBuffer* linkedBuffer = new LinkedBuffer(std::move(buffer));
		linkedBuffer->retrieve(nwrote);
		append(linkedBuffer);
	}
}

void SendBuffer::append(LinkedBuffer* buffer) {
	if (tail_) {
		tail_->next_ = buffer;
		tail_ = tail_->next_;
	} else {
		tail_ = head_ = buffer;
	}
	++size_;
}

int SendBuffer::fill(struct iovec* iov, int iovcnt) {
	LinkedBuffer* buffer = head_;
	uint32_t count = 0;
	int i = 0;
	while (buffer && i < iovcnt && count < kMaxWrite) {
		uint32_t writable = buffer->write_ - buffer->read_;
		iov[i].iov_base = buffer->buffer_ + buffer->read_;
		iov[i].iov_len = writable;
		count += writable;
		++i;
		buffer = buffer->next_;
	}
	return i;
}

void SendBuffer::retrieve(uint32_t count) {
	uint32_t remain = count;
	while (remain > 0) {
		LinkedBuffer* buffer = head_;
		uint32_t writable = buffer->write_ - buffer->read_;
		if (remain < writable) {
			buffer->retrieve(remain);
			remain = 0;
		} else {
			if (buffer->next_) {
				head_ = buffer->next_;
			} else {
				head_ = tail_ = nullptr;
			}
			delete buffer;
			--size_;
			remain -= writable;
		}
	}
	count_ -= count;
}

*/

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
		return new Buffer(1024 * 8);
	}
}

SendBuffer::SendBuffer(BufferPool* pool)
	: pool_(pool)
	, listBuffer_()
	, count_(0) {
}

SendBuffer::~SendBuffer() {
}

void SendBuffer::write(const char* buf, uint32_t count) {
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
		std::memcpy(buffer->data(), buf + nwrote, writable);
		buffer->hasWritten(writable);

		listBuffer_.pushBack(buffer);
		nwrote += writable;
	}
	count_ += count;
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

void SendBuffer::retrieve(uint32_t count) {
	/*
	uint32_t writable = std::min(buffer->write_ - buffer->read_, count);
	buffer->retrieve(writable);
	uint32_t nwrote = writable;
	while (nwrote < count) {

	}

	uint32_t remain = count;
	while (remain > 0) {
		Buffer* buffer = head_;
		uint32_t writable = buffer->write_ - buffer->read_;
		if (remain < writable) {
			buffer->retrieve(remain);
			remain = 0;
		} else {
			if (buffer->next_) {
				head_ = buffer->next_;
			} else {
				head_ = tail_ = nullptr;
			}
			delete buffer;
			--size_;
			remain -= writable;
		}
	}
	count_ -= count;*/
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