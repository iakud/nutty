#include <catta/net/Buffer.h>

#include <catta/net/Socket.h>

using namespace catta;

const size_t Buffer::kCapacity;

Buffer::Buffer()
	: capacity_(kCapacity)
	, count_(0)
	, write_(0)
	, read_(0)
	, next_(nullptr) {

}

Buffer::~Buffer() {

}

ListBuffer::ListBuffer() {

}

ssize_t ListBuffer::write(Socket& socket) {

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
	return readv(socket);
}

ssize_t ListBuffer::readv(Socket& socket) {
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
		//next_ = 
	}
	iov[iovcnt].iov_base = next_->buffer_;
	iov[iovcnt].iov_len = next_->capacity_;
	++iovcnt; // iovcnt <= 3
	
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
	}
	return nread;
}