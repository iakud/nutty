#include <nutty/log/LogStream.h>

#include <type_traits>
#include <algorithm>
#include <limits>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

namespace nutty {

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof digits == 20, "sizeof digits == 20");

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof digitsHex == 17, "sizeof digitsHex == 17");

// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
size_t convert(char buf[], T value) {
	T i = value;
	char* p = buf;

	do {
		int lsd = static_cast<int>(i % 10);
		i /= 10;
		*p++ = zero[lsd];
	} while (i != 0);

	if (value < 0) {
		*p++ = '-';
	}

	*p = '\0';
	std::reverse(buf, p);
	return p - buf;
}

size_t convertHex(char buf[], uintptr_t value) {
	uintptr_t i = value;
	char* p = buf;

	do {
		int lsd = i % 16;
		i /= 16;
		*p++ = digitsHex[lsd];
	} while (i != 0);

	*p = '\0';
	std::reverse(buf, p);
	return p - buf;
}

} // end namespace nutty

using namespace nutty;

void LogStream::static_check() {
	static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10, "kMaxNumericSize - 10 > std::numeric_limits<double>::digits10");
	static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10, "kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10");
	static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10, "kMaxNumericSize - 10 > std::numeric_limits<long>::digits10");
	static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10, "kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10");
}

LogStream& LogStream::operator<<(short v) {
	formatInteger(static_cast<int>(v));
	return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
	formatInteger(static_cast<unsigned int>(v));
	return *this;
}

LogStream& LogStream::operator<<(int v) {
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long v) {
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long long v) {
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
	formatInteger(v);
	return *this;
}

template<typename T>
void LogStream::formatInteger(T v) {
	uint32_t needSize = buffer_.size() + kMaxNumericSize;
	if (needSize > buffer_.capacity()) {
		buffer_.reserve(needSize);
	}
	buffer_.add(convert(buffer_.current(), v));
}

// FIXME: replace this with Grisu3 by Florian Loitsch.
LogStream& LogStream::operator<<(double v) {
	uint32_t needSize = buffer_.size() + kMaxNumericSize;
	if (needSize > buffer_.capacity()) {
		buffer_.reserve(needSize);
	}
	buffer_.add(::snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v));
	return *this;
}

LogStream& LogStream::operator<<(const void* p) {
	uintptr_t v = reinterpret_cast<uintptr_t>(p);

	uint32_t needSize = buffer_.size() + kMaxNumericSize;
	if (needSize > buffer_.capacity()) {
		buffer_.reserve(needSize);
	}
	char* buf = buffer_.current();
	buf[0] = '0';
	buf[1] = 'x';
	buffer_.add(2);
	buffer_.add(convertHex(buffer_.current(), v));
	return *this;
}