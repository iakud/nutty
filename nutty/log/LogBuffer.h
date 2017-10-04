#ifndef NUTTY_LOG_LOGBUFFER_H
#define NUTTY_LOG_LOGBUFFER_H

namespace nutty {

class LogBuffer {
public:
	LogBuffer();
	~LogBuffer();

	inline char* current() { return buf_ + size_; }
	inline void add(std::size_t size) { size_ += size; }

	inline std::size_t size() const { return size_; }
	inline std::size_t capacity() const { return capacity_; }

	void reserve(std::size_t capacity);

	void append(void* buf, std::size_t count) {
		std::size_t newSize = size_ + count;
		if (newSize > capacity_) {
			reserve(newSize);
		}
		std::memcpy(buf_ + size_, buf, count);
		size_ = newSize;
	}

private:
	static const int kBufferSize = 512;

	char* buf_;
    std::size_t size_;
    std::size_t capacity_;
}; // end class LogBuffer

} // end namesapce nutty

#endif // NUTTY_LOG_BUFFER_H