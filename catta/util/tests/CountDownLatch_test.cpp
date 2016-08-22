#include <catta/util/CountDownLatch.h>

#include <thread>
#include <vector>
#include <string>

#include <iostream>

class Test {
public:
	Test(int numThreads) : latch_(numThreads) {
		for (int i = 0; i < numThreads; ++i) {
			threads_.emplace_back(std::bind(&Test::threadFunc, this));
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
	void threadFunc() {
		std::cout << "thread " << std::this_thread::get_id() << " started" << std::endl;
		latch_.countDown();
		std::cout << "thread " << std::this_thread::get_id() << " stopped" << std::endl;
	}

	catta::CountDownLatch latch_;
	std::vector<std::thread> threads_;
};

int main() {
	Test test(10);
	test.wait();
}