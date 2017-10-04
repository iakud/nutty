#include <chrono>
#include <memory>
#include <iostream>
#include <sstream>
#include <cstring>
#include <ostream>


const int kBufferSize = 4 * 1024;
const int kCount = 9999;

char buffer[4*1024*1024];

void stack() {
	//char buf[kBufferSize*100];
	//(void)buf;
	//std::ostringstream oss;
	//oss << "123" << 1.123;
	//std::string s;
	//oss.str(s);

	//std::string s(oss.str());
	//(void)s;
	//(void)oss;
	char buf[4*1024];
	std::memcpy(buf, "123123123123123123123123123123", 25);
	/*
	std::memcpy(buf, "123123123123123123123123123123", 25);
	std::memcpy(buf, "123123123123123123123123123123", 25);
	std::memcpy(buf, "123123123123123123123123123123", 25);
	std::memcpy(buf, "123123123123123123123123123123", 25);
	std::memcpy(buf, "123123123123123123123123123123", 25);
	std::memcpy(buf, "123123123123123123123123123123", 25);
	std::memcpy(buf, "123123123123123123123123123123", 25);
	*/
	std::memcpy(buffer, buf, 512);
}


class LogStreamBuf : public std::streambuf {
 public:
  // REQUIREMENTS: "len" must be >= 2 to account for the '\n' and '\n'.
  LogStreamBuf(char *buf, int len) {
    setp(buf, buf + len - 2);
  }
  // This effectively ignores overflow.
  virtual int_type overflow(int_type ch) {
    return ch;
  }

  // Legacy public ostrstream method.
  size_t pcount() const { return pptr() - pbase(); }
  char* pbase() const { return std::streambuf::pbase(); }
};

void heap() {
//	char buf[10];
//	(void)buf;
//	char* buf = static_cast<char*>(std::malloc(kBufferSize));
	//(void)buf;
//	std::free(buf);
	//char* buf = static_cast<char*>(std::malloc(1024));
	//std::memcpy(buf, "123123123123123123123123123123", 25);
	//std::ostringstream oss;
	//oss << "123123123123123123123123123123";// << 1.123;
	/*
	oss << "123123123123123123123123123123";
	oss << "123123123123123123123123123123";
	oss << "123123123123123123123123123123";
	oss << "123123123123123123123123123123";
	oss << "123123123123123123123123123123";
	oss << "123123123123123123123123123123";
	oss << "123123123123123123123123123123";
	*/
	//std::string s("123123123123123123123123123123");
	//s += "123123123123123123123123123123";
	//std::string s;
	//oss.str(s);
	//(void)s;
	//buffs.push_back(buf);
	//std::free(buf);
	char buf[4*1024];
	LogStreamBuf sb(buf, 4*1024);
	std::ostream os(&sb);
	os << "123123123123123123123123123123";
	/*
	std::ostringstream oss;
	oss << "123123123123123123123123123123";
	*/
}

int main() {
	stack();

	std::chrono::system_clock::time_point t1 = std::chrono::system_clock::now();
	for (int i = 0; i < kCount; ++i) {
		stack();
	}
	std::chrono::system_clock::time_point t2 = std::chrono::system_clock::now();
	std::cout << "stack:\t"<< (t2-t1).count() << std::endl;

	std::chrono::system_clock::time_point t3 = std::chrono::system_clock::now();
	for (int i = 0; i < kCount; ++i) {
		heap();
		
	}
	std::chrono::system_clock::time_point t4 = std::chrono::system_clock::now();
	std::cout << "heap:\t"<< (t4-t3).count() << std::endl;
	return 0;
}