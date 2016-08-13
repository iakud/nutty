#include <catta/util/singleton.h>

#include <thread>
#include <iostream>

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
	std::string name_;
};

class TestNoDestroy {
public:
	// tag no_destroy for singleton<T>
	void no_destroy();

	TestNoDestroy() {
		std::cout << "constructing no destroy " << this << std::endl;
	}

	~TestNoDestroy() {
		std::cout << "destructing no destroy " << this << std::endl;
	}
};

void threadFunc() {
	catta::singleton<Test>::instance().setName("two");
}

int main() {
	catta::singleton<Test>::instance().setName("one");
	std::thread t(threadFunc);
	t.join();
	std::cout << "name = " <<  catta::singleton<Test>::instance().getName() << std::endl;
	catta::singleton<TestNoDestroy>::instance();
	return 0;
}
