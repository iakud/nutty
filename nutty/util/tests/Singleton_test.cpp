#include <nutty/util/Singleton.h>

#include <thread>
#include <iostream>

using namespace nutty;

class Test {
public:
	Test() {
		std::cout << "constructing " << this << std::endl;
	}

	~Test() {
		std::cout << "destructing " << this << " " << name_ << std::endl;
	}

	const std::string& getName() const { return name_; }
	void setName(const std::string& name) { name_ = name; }

private:
	Test(const Test&) = delete;
	Test& operator=(const Test&) = delete;

	std::string name_;
};

class TestNoDestroy {
public:
	TestNoDestroy() {
		std::cout << "constructing no destroy " << this << std::endl;
	}

	~TestNoDestroy() {
		std::cout << "destructing no destroy " << this << std::endl;
	}

	// tag no_destroy for Singleton<T>
	void no_destroy();

private:
	TestNoDestroy(const TestNoDestroy&) = delete;
	TestNoDestroy& operator=(const TestNoDestroy&) = delete;
};

void threadFunc() {
	Singleton<Test>::instance().setName("two");
}

int main() {
	Singleton<Test>::instance().setName("one");
	std::thread t(threadFunc);
	t.join();
	std::cout << "name = " <<  Singleton<Test>::instance().getName() << std::endl;
	Singleton<TestNoDestroy>::instance();
	return 0;
}
