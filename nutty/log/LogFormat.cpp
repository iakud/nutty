#include <nutty/log/LogFormat.h>

#include <algorithm>

#include <cassert>
#include <cstdint>

using namespace nutty;

template<typename T>
LogFormat::LogFormat(const char* fmt, T val) {
	static_assert(std::is_arithmetic<T>::value, "std::is_arithmetic<T>::value");
	length_ = ::snprintf(data_, sizeof data_, fmt, val);
	assert(static_cast<size_t>(length_) < sizeof data_);
}

// explicit instantiations
template LogFormat::LogFormat(const char* fmt, char);
template LogFormat::LogFormat(const char* fmt, unsigned char);

template LogFormat::LogFormat(const char* fmt, short);
template LogFormat::LogFormat(const char* fmt, unsigned short);
template LogFormat::LogFormat(const char* fmt, int);
template LogFormat::LogFormat(const char* fmt, unsigned int);
template LogFormat::LogFormat(const char* fmt, long);
template LogFormat::LogFormat(const char* fmt, unsigned long);
template LogFormat::LogFormat(const char* fmt, long long);
template LogFormat::LogFormat(const char* fmt, unsigned long long);

template LogFormat::LogFormat(const char* fmt, float);
template LogFormat::LogFormat(const char* fmt, double);