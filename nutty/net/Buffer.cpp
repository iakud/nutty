#include <nutty/net/Buffer.h>

#include <cstring>
#include <memory>

using namespace nutty;

Buffer::Buffer(size_t capacity)
	: buffer_(capacity)
	, rpos_(0)
	, wpos_(0) {
}

Buffer::Buffer(const void* buf, size_t count)
	: buffer_(count)
	, rpos_(0)
	, wpos_(count) {
	std::memcpy(buffer_.data(), buf, count);
}

Buffer::Buffer(Buffer&& buffer)
	: buffer_(std::move(buffer.buffer_))
	, rpos_(buffer.rpos_)
	, wpos_(buffer.wpos_) {
	buffer.rpos_ = buffer.wpos_ = 0;
}