#include <catta/util/singleton.h>

#include <thread>
#include <iostream>

class test {
public:
	test() {
		std::cout << "constructing " << this << std::endl;
	}

	~test() {
		std::cout << "destructing " << this << " " << name_ << std::endl;
	}

	const std::string& getName() const { return name_; }
	void setName(const std::string& name) { name_ = name; }

public:
	// noncopyable
	test(const test&) = delete;
	test& operator=(const test&) = delete;

private:
	std::string name_;
};

class test_no_destroy {
public:
	test_no_destroy() {
		std::cout << "constructing no destroy " << this << std::endl;
	}

	~test_no_destroy() {
		std::cout << "destructing no destroy " << this << std::endl;
	}

	// tag no_destroy for singleton<T>
	void no_destroy();

public:
	// noncopyable
	test_no_destroy(const test_no_destroy&) = delete;
	test_no_destroy& operator=(const test_no_destroy&) = delete;
};

void threadFunc() {
	catta::singleton<test>::instance().setName("two");
}

int main() {
	catta::singleton<test>::instance().setName("one");
	std::thread t(threadFunc);
	t.join();
	std::cout << "name = " <<  catta::singleton<test>::instance().getName() << std::endl;
	catta::singleton<test_no_destroy>::instance();
	return 0;
}
