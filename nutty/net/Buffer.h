#ifndef NUTTY_NET_BUFFER_H
#define NUTTY_NET_BUFFER_H

#include <cstdint>

namespace nutty {

class Buffer {
public:
	Buffer(uint32_t capacity);
	Buffer(const void* buf, uint32_t count);
	Buffer(Buffer&& buffer);
	~Buffer();

	inline char* data() const { return buf_; }
	inline char* dataRead() const { return buf_ + rpos_; }
	inline char* dataWrite() const { return buf_ + wpos_; }

	inline uint32_t capacity() const { return cap_; }
	inline uint32_t readableSize() const { return wpos_ - rpos_; }
	inline uint32_t writableSize() const { return cap_ - wpos_; }

	inline void hasRead(uint32_t count) { rpos_ += count; }
	inline void hasWritten(uint32_t count) { wpos_ += count; }
	inline void reset() { rpos_ = wpos_ = 0; }
	inline bool empty() const { return rpos_ == wpos_; }
	inline bool full() const { return wpos_ == cap_; }

private:
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	char* buf_;
	uint32_t cap_;
	uint32_t rpos_;
	uint32_t wpos_;
}; // end class Buffer

} // end namespace nutty

#endif // NUTTY_NET_BUFFER_H