#include <catta/util/Singleton.h>

#include <catta/base/noncopyable.h>

#include <thread>
#include <iostream>

class Test : catta::noncopyable {
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
	std::string name_;
};

class TestNoDestroy : catta::noncopyable {
public:
	TestNoDestroy() {
		std::cout << "constructing no destroy " << this << std::endl;
	}

	~TestNoDestroy() {
		std::cout << "destructing no destroy " << this << std::endl;
	}

	// tag no_destroy for Singleton<T>
	void no_destroy();
};

void threadFunc() {
	catta::Singleton<Test>::instance().setName("two");
}

int main() {
	catta::Singleton<Test>::instance().setName("one");
	std::thread t(threadFunc);
	t.join();
	std::cout << "name = " <<  catta::Singleton<Test>::instance().getName() << std::endl;
	catta::Singleton<TestNoDestroy>::instance();
	return 0;
}
