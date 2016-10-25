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

ListBuffer::ListBuffer(BufferPool* pool)
	: pool_(pool) {
}

ListBuffer::~ListBuffer() {
}

void ListBuffer::write(const char* buf, size_t count) {
	size_t writable = tail_->capacity_ - tail_->write_;
	if (count < writable) {
		std::memcpy(tail_->buffer_ + tail_->write_, buf, count);
		tail_->write_ += count;
	} else {
		std::memcpy(tail_->buffer_ + tail_->write_, buf, writable);
		tail_->write_ += writable;
		size_t nwrite = writable;
		while (nwrite < count) {
			tail_->next_ = pool_->take();
			tail_ = tail_->next_;
			size_t size = count - nwrite < tail_->capacity_ ? count - nwrite : tail_->capacity_;
			std::memcpy(tail_->buffer_, buf + nwrite, size);
			tail_->write += size;
			nwrite += size;
		}
		count_ += count;
	}
}

void ListBuffer::read(char* data, size_t count) {
	
}

ssize_t ListBuffer::write(Socket& socket) {
	if (!head_->next_) {
		const ssize_t nwrite = socket.write(head_->buffer_ + head_->read_, head_->count_);
		if (nwrite > 0) {
			if (nwrite < head_->count_) {
				head_->read_ += nwrite;
				head_->count_ -= nwrite;
			} else {
				head_->count_ = head_->read_ = head_->write_ = 0;
			}			
			count_ -= nwrite;
		}
		return nwrite;
	}

	if (head_->read_ == 0) {
		const ssize_t nwrite = socket.write(head_->buffer_, head_->count_);
		if (nwrite > 0) {
			if (nwrite < head_->count_) {
				head_->read_ += nwrite;
				head_->count_ -= nwrite;
			} else {
				head_->count_ = head_->read_ = head_->write_ = 0;
				Buffer* next = head_->next_;
				head_->next_ = nullptr;
				pool_->put(head_);
				head_ = next;
				if (static_cast<size_t>(nwrite) > head_->count_) {
					head_->read_ = nwrite - head_->count_;
					head_->count_ -= next->read_;
				}
			}
			count_ -= nwrite;
		}
		return nwrite;
	}

	struct iovec iov[2];
	Buffer* next = head_->next_;
	size_t readable = head_->count_;
	iov[0].iov_base = head_->buffer_ + head_->read_;
	iov[0].iov_len = head_->capacity_ - head_->read_;
	iov[1].iov_base = next->buffer_;
	iov[1].iov_len = next->count_;

	const ssize_t nwrite = socket.writev(iov, 2);
	if (nwrite > 0) {
		if (static_cast<size_t>(nwrite) < readable) {
			head_->read_ += nwrite;
			head_->count_ -= nwrite;
		} else {
			head_->count_ = head_->read_ = head_->write_ = 0;
			head_->next_ = nullptr;
			pool_->put(head_);
			head_ = next;
			if (static_cast<size_t>(nwrite) > readable) {
				head_->read_ = nwrite - readable;
				head_->count_ -= next->read_;
			}
		}
		count_ -= nwrite;
	}
	return nwrite;
}

ssize_t ListBuffer::read(Socket& socket) {
	if (tail_->write_ == 0) {
		const ssize_t nread = socket.read(tail_->buffer_, tail_->capacity_);
		if (nread > 0) {
			tail_->write_ = nread;
			tail_->count = nread;
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
			tail_->count_ = tail_->write_ = nread;
			count_ += nread;
		}
		return nread;
	}

	struct iovec iov[2];
	Buffer* next = pool_->next();
	size_t writable = tail_->capacity_ - tail_->write_;
	iov[0].iov_base = tail_->buffer_ + tail_->write_;
	iov[0].iov_len = tail_->capacity_ - tail_->write_;
	iov[1].iov_base = next->buffer_;
	iov[1].iov_len = next->capacity_;

	const ssize_t nread = socket.readv(iov, 2);
	if (nread > 0) {
		if (static_cast<size_t>(nread) < writable) {
			tail_->write_ += nread;
			tail_->count_ += nread;
		} else {
			tail_->write_ = tail_->capacity_;
			tail_->count_ += nread;
			if (static_cast<size_t>(nread) > writable)  {
				tail_->next_ = next;
				tail_ = next;
				tail_->count_ = tail_->write_ = nread - writable;
			}
		}
		count_ += nread;
	}
}

/*
ssize_t ListBuffer::write(Socket& socket) {
	struct iovec iov[3];
	int iovcnt = 0;
	size_t readable = head_->count;
	if (head_->read_ < head_->write_) { // less(read<write)
		iov[iovcnt].iov_base = head_->buffer_ + head_->read_;
		iov[iovcnt].iov_len = readable;
		++iovcnt; // iovcnt = 1
	} else { // more(read>write) and full(read==write)
		iov[iovcnt].iov_base = head_->buffer_ + head_->read_;
		iov[iovcnt].iov_len = head_->capacity_ - head_->read;
		++iovcnt; // iovcnt = 1
		if (head_->push > 0) {
			iov[iovcnt].iov_base = head_->buffer_;
			iov[iovcnt].iov_len = head_->write_;
			++iovcnt; // iovcnt = 2
		}
	}

	size_t size = readable;
	Buffer* next = head_->next_;
	if (next) {
		iov[iovcnt].iov_base = next->buffer_;
		iov[iovcnt].iov_len = next->count_;
		++iovcnt; // iovcnt <= 3
		size += next->count_;
	}

	const ssize_t nwrite = socket.writev(iov, iovcnt);
	if (nwrite > 0) {
		if (nwrite < readable) {
			if (head_->read_ + nwrite > head_->capacity_) {
				head_->read_ = head_->read_ + nwrite - head_->capacity_;
			} else {
				head_->read_ += nwrite;
			}
			head_->count_ -= nwrite;
		} else if (nwrite == readable) {
			if (next) {// next
				head_->next_ = nullptr;
				pool_->put(head_);
				head_ = next;
			} else {
				head_->write_ = head_->read_ = head_->count_ = 0;
			}
		} else {
			next->read_ = nwrite - readable;
			next->count_ -= next->read_;
			pool_->put(head_);
			head_ = next;
		}

		count_ -= nwrite;
		if (nwrite < size) {

		}
	}

	return 0;
}

ssize_t ListBuffer::read(Socket& socket) {
	if (current_->empty()) {
		const ssize_t nread = socket.read(current_->buffer_, current_->capacity_);
		if (nread > 0) {
			current_->write_ = nread;
			count_ += nread;
		}
		return nread;
	}

	struct iovec iov[3];
	int iovcnt = 0;
	size_t writable = current_->capacity_ - current_->count_;
	if (writable > 0) { // current buffer
		if (current_->write_ < current_->read_) { // less(write<read)
			iov[iovcnt].iov_base = current_->buffer_ + current_->write_;
			iov[iovcnt].iov_len = current_->capacity_ - current_->count_;
			++iovcnt; // iovcnt = 1
		} else { // more(write>read) and empty(write==read)
			iov[iovcnt].iov_base = current_->buffer_ + current_->write_;
			iov[iovcnt].iov_len = current_->capacity_ - current_->write_;
			++iovcnt; // iovcnt = 1
			if (current_->read_ > 0) {
				iov[iovcnt].iov_base = current_->buffer_;
				iov[iovcnt].iov_len = current_->read_;
				++iovcnt; // iovcnt = 2
			}
		}
	}

	if (!next_) {
		next_ = pool_->take();
	}
	iov[iovcnt].iov_base = next_->buffer_;
	iov[iovcnt].iov_len = next_->capacity_;
	++iovcnt; // iovcnt <= 3

	size_t size = writable + next_->capacity_;

	const ssize_t nread = socket.readv(iov, iovcnt);
	if (nread > 0) {
		if (static_cast<size_t>(nread) < writable) {
			if (current_->write_ + nread > current_->capacity_) {
				current_->write_ = current_->write_ + nread - current_->capacity_;
			} else {
				current_->write_ += nread;
			}
			current_->count_ += nread;
		} else {
			current_->write_ = current_->read_;
			current_->count_ = current_->capacity_;
			if (static_cast<size_t>(nread) > writable) {// fill next
				next_->count_ = next_->write_ = nread - writable;
				current_->next_ = next_;
				current_ = next_;
				next_ = nullptr;
			}
		}
		count_ += nread;

		if (nread < size) {
			// FIXME :
		}
	}
	return nread;
}
*/
