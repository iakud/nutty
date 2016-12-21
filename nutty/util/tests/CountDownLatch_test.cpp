#include <nutty/util/CountDownLatch.h>

#include <thread>
#include <vector>
#include <string>

#include <iostream>

using namespace nutty;

class Test {
public:
	Test(int numThreads)
		: latch_(numThreads)
		, threads_() {
		for (int i = 0; i < numThreads; ++i) {
			threads_.emplace_back(&Test::threadFunc, this);
		}
	}

	~Test() {
		for (auto& t : threads_) {
			t.join();
		}
	}

	void wait() {
		std::cout << "waiting for count down latch" << std::endl;
		latch_.wait();
		std::cout << "all threads started" << std::endl;
	}

private:
	Test(const Test&) = delete;
	Test& operator=(const Test&) = delete;
	
	void threadFunc() {
		std::cout << "thread " << std::this_thread::get_id() << " started" << std::endl;
		latch_.countDown();
		std::cout << "thread " << std::this_thread::get_id() << " stopped" << std::endl;
	}

	CountDownLatch latch_;
	std::vector<std::thread> threads_;
};

int main() {
	Test test(10);
	test.wait();
}