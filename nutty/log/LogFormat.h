#ifndef NUTTY_LOG_LOGFORMAT_H
#define NUTTY_LOG_LOGFORMAT_H

namespace nutty {

class LogFormat {
private:
	static const int kBufferSize = 32;

public:
	template<typename T>
	LogFormat(const char* fmt, T val);

	const char* data() const { return data_; }
	int length() const { return length_; }

private:
	char data_[kBufferSize];
	int length_;
}; // end class LogFormat

} // end namespace nutty

#endif // NUTTY_LOG_LOGFORMAT_H