#ifndef NUTTY_NET_BUFFER_H
#define NUTTY_NET_BUFFER_H

#include <vector>
#include <cstddef>

namespace nutty {

class Buffer {
public:
	Buffer(size_t capacity);
	Buffer(const void* buf, size_t count);
	Buffer(Buffer&& buffer);

	inline char* data() { return buffer_.data(); }
	inline char* dataRead() { return buffer_.data() + rpos_; }
	inline char* dataWrite() { return buffer_.data() + wpos_; }

	inline size_t capacity() const { return buffer_.capacity(); }
	inline size_t readableSize() const { return wpos_ - rpos_; }
	inline size_t writableSize() const { return buffer_.capacity() - wpos_; }

	inline void hasRead(size_t count) { rpos_ += count; }
	inline void hasWritten(size_t count) { wpos_ += count; }
	inline void reset() { rpos_ = wpos_ = 0; }
	inline bool empty() const { return rpos_ == wpos_; }
	inline bool full() const { return wpos_ == buffer_.capacity(); }

private:
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	std::vector<char> buffer_;
	size_t rpos_;
	size_t wpos_;
}; // end class Buffer

} // end namespace nutty

#endif // NUTTY_NET_BUFFER_H