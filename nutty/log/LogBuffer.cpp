#include <nutty/log/LogBuffer.h>

using namespace nutty;

LogBuffer::LogBuffer()
	: size_(0)
	, capacity_(kBufferSize) {
	buf_ = static_cast<char*>(std::malloc(kBufferSize));
}

LogBuffer::~LogBuffer() {
	if (buf_) {
		std::free(buf_);
	}
}

void LogBuffer::reserve(std::size_t capacity) {
	std::size_t new_capacity = std::max(capacity_ + capacity_ / 2, capacity);
	char* buf = static_cast<char*>(std::malloc(new_capacity));
	std::memcpy(buf, buf_, size_);
	std::free(buf_);
	buf_ = buf;
	capacity_ = new_capacity;
}