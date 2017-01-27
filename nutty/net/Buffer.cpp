#include <nutty/net/Buffer.h>

#include <cstring>
#include <memory>

using namespace nutty;

Buffer::Buffer(uint32_t capacity)
	: cap_(capacity)
	, rpos_(0)
	, wpos_(0) {
	buf_ = static_cast<char*>(std::malloc(cap_));
}

Buffer::Buffer(const void* buf, uint32_t count)
	: cap_(count)
	, rpos_(0)
	, wpos_(count) {
	buf_ = static_cast<char*>(std::malloc(cap_));
	std::memcpy(buf_, buf, count);
}

Buffer::Buffer(Buffer&& buffer)
	: buf_(buffer.buf_)
	, cap_(buffer.cap_)
	, rpos_(buffer.rpos_)
	, wpos_(buffer.wpos_) {
	buffer.buf_ = nullptr;
	buffer.cap_ = buffer.rpos_ = buffer.wpos_ = 0;
}

Buffer::~Buffer() {
	if (buf_) {
		std::free(buf_);
	}
}