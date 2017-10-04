#ifndef NUTTY_LOG_LOGSTREAM_H
#define NUTTY_LOG_LOGSTREAM_H

#include <string>
#include <memory.h> // memcpy

namespace nutty {

class LogStream {
public:
	LogStream& operator<<(short v);
	LogStream& operator<<(unsigned short v);
	LogStream& operator<<(int v);
	LogStream& operator<<(unsigned int v);
	LogStream& operator<<(long v);
	LogStream& operator<<(unsigned long v);
	LogStream& operator<<(long long v);
	LogStream& operator<<(unsigned long long v);

	LogStream& operator<<(float v) { *this << static_cast<double>(v); return *this; }
	LogStream& operator<<(double v);
	LogStream& operator<<(char v) { append(&v, 1); return *this; }
	LogStream& operator<<(unsigned char v) { append(&v, 1); return *this; }
	LogStream& operator<<(const char* v) { append(v, strlen(v)); return *this; }
	LogStream& operator<<(const std::string& v) { append(v.c_str(), v.size()); return *this; }
	LogStream& operator<<(bool v) { append(v ? "1" : "0", 1); return *this; }
	LogStream& operator<<(const void*);

	const char* data() const { return data_; }

	void append(const char* buf, int count) { buffer_.append(buf, count); }
	const LogBuffer& buffer() const { return buffer_; }

private:
	static const int kMaxNumericSize = 32;
	// noncopyable
	LogStream(const LogStream&) = delete;
	LogStream& operator=(const LogStream&) = delete;

	void static_check(); // for static_assert

	template<typename T>
	void formatInteger(T);

	LogBuffer buffer_;
}; // end class LogStream

inline LogStream& operator<<(LogStream& stream, const LogFormat& fmt) {
	stream.append(fmt.data(), fmt.length());
	return stream;
}

} // end namespace nutty

#endif  // NUTTY_LOG_LOGSTREAM_H

