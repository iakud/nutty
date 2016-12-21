#include <nutty/util/Semaphore.h>

#include <thread>

#include <iostream>

using namespace nutty;

class Test {
public:
	Test()
		: sem_()
		, thread_() {
		thread_ = std::thread(&Test::threadFunc, this);
	}

	~Test() {
		thread_.join();
	}

	void wait() {
		std::cout << "wait for signal" << std::endl;
		sem_.wait();
		std::cout << "recv signal" << std::endl;
		sem_.wait();
		std::cout << "recv signal" << std::endl;

		if (sem_.try_wait()) {
			std::cout << "try_wait recv signal" << std::endl;
		} else {
			std::cout << "try_wait recv failed" << std::endl;
		}

		if (sem_.wait_for(std::chrono::milliseconds(5))) {
			std::cout << "wait_for recv signal" << std::endl;
		} else {
			std::cout << "wait_for recv timeout" << std::endl;
		}

		if (sem_.wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10))) {
			std::cout << "wait_until recv signal" << std::endl;
		} else {
			std::cout << "wait_until recv timeout" << std::endl;
		}
	}

private:
	Test(const Test&) = delete;
	Test& operator=(const Test&) = delete;
	
	void threadFunc() {
		std::cout << "send signal" << std::endl;
		sem_.notify();
		std::cout << "send signal" << std::endl;
		sem_.notify();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::cout << "send signal" << std::endl;
		sem_.notify();
	}

	Semaphore sem_;
	std::thread thread_;
};

int main() {
	Test test;
	test.wait();
}