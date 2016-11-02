#include <catta/net/Buffer.h>

#include <catta/net/Socket.h>

#include <cstring>

using namespace catta;
Buffer::Buffer(uint32_t capacity)
	: capacity_()
	, readIndex_(0)
	, writeIndex_(0)
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
	, count_(0)
	, head_(nullptr)
	, tail_(nullptr) {
}

BufferPool::~BufferPool() {
	Buffer* buffer = take();
	while (buffer) {
		delete buffer;
		buffer = take();
	}
}

void BufferPool::put(Buffer* buffer) {
	if (buffer) {
		if (count_ < capacity_) {
			buffer->next_ = nullptr;
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
}

Buffer* BufferPool::take() {
	Buffer* buffer;
	if (count_ > 0) {
		buffer = head_;
		if (head_ == tail_) {
			head_ = tail_ = nullptr;
		} else {
			head_ = head_->next_;
			buffer->next_ = nullptr;
		}
		--count_;
	} else {
		buffer = new Buffer(1024 * 8);
	}
	return buffer;
}

SendBuffer::SendBuffer(BufferPool* pool)
	: pool_(pool)
	, size_(0)
	, count_(0)
	, head_(0)
	, tail_(0) {
}

void SendBuffer::write(const void* buf, uint32_t count) {
	/*
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
	*/
}

int SendBuffer::prepareSend(struct iovec* iov, int iovcnt) {
	/*
	Buffer* buffer = listBuffer_.front();
	uint32_t size = 0;
	int i = 0;
	while (buffer && i < iovcnt && size < kMaxSend) {
		uint32_t readable = buffer->readableBytes();
		iov[i].iov_base = buffer->dataRead();
		iov[i].iov_len = readable;
		buffer = buffer->next();
		++i;
		size += readable;
	}
	return i;
	*/
	return 0;
}

void SendBuffer::hasSent(uint32_t count) {
	/*
	uint32_t readable;
	uint32_t nsent = 0;
	while (nsent < count) {
		Buffer* buffer = listBuffer_.front();
		readable = std::min(buffer->readableBytes(), count - nsent);
		buffer->hasRead(readable);
		if (buffer->readableBytes() == 0) {
			pool_->put(listBuffer_.popFront());
		}
		nsent += readable;
	}
	size_ -= count;
	*/
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

ReceiveBuffer::ReceiveBuffer(BufferPool* pool)
	: pool_(pool)
	, size_(0)
	, count_(0)
	, head_(nullptr)
	, tail_(nullptr) {
}

ReceiveBuffer::~ReceiveBuffer() {
	while (head_) {
		Buffer* head = head_;
		head_ = head->next_;
		pool_->put(head);
	}
}

int ReceiveBuffer::prepareReceive(struct iovec* iov, int iovcnt) {
	int count = 0;
	uint32_t receiveSize = 0;
	uint32_t writableSize = tail_->capacity_ - tail_->count_;
	if (writableSize > 0) {
		if (tail_->writeIndex_ < tail_->readIndex_) {
			iov[count].iov_base = tail_->buffer_ + tail_->writeIndex_;
			iov[count].iov_len = writableSize;
			++count;
		} else {
			iov[count].iov_base = tail_->buffer_ + tail_->writeIndex_;
			iov[count].iov_len = tail_->capacity_ - tail_->writeIndex_;
			++count;
			if (tail_->readIndex_ > 0) {
				iov[count].iov_base = tail_->buffer_;
				iov[count].iov_len = tail_->readIndex_;
				++count;
			}
		}
	}
	receiveSize += writableSize;
	Buffer* buffer = tail_;
	while (receiveSize < kMaxReceive) {
		buffer->next_ = pool_->take();
		buffer = buffer->next_;
		iov[count].iov_base = buffer->buffer_;
		iov[count].iov_len = buffer->capacity_;
		++count;
		receiveSize += buffer->capacity_;
	}
	return count;
}

void ReceiveBuffer::hasReceived(uint32_t count) {
	uint32_t writableSize = tail_->capacity_ - tail_->count_;
	if (count < writableSize) {
		uint32_t writeIndex = tail_->writeIndex_ + count;
		tail_->writeIndex_ = writeIndex < tail_->capacity_ ? writeIndex : writeIndex - tail_->capacity_;
		tail_->count_ += count;
	} else {
		tail_->writeIndex_ = tail_->readIndex_;
		tail_->count_ = tail_->capacity_;
		uint32_t receiveCount = count - writableSize;
		while (receiveCount > 0) {
			tail_ = tail_->next_;
			if (receiveCount < tail_->capacity_) {
				tail_->count_ = tail_->writeIndex_ = receiveCount;
				receiveCount = 0;
			} else {
				tail_->count_ = tail_->capacity_;
				receiveCount -= tail_->capacity_;
			}
		}
	}
	size_ -= count;
}