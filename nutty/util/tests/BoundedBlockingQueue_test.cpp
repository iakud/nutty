#include <nutty/util/BoundedBlockingQueue.h>
#include <nutty/util/CountDownLatch.h>

#include <thread>
#include <vector>
#include <string>
#include <iostream>

using namespace nutty;

class Test {
public:
	Test(int numThreads)
		: queue_(20)
		, latch_(numThreads)
		, threads_() {
		for (int i = 0; i < numThreads; ++i) {
			threads_.emplace_back(&Test::threadFunc, this);
		}
	}

	~Test() {
		for (size_t i = 0; i < threads_.size(); ++i) {
			queue_.put("stop");
		}
		for (auto& t : threads_) {
			t.join();
		}
	}

	void run(int times) {
		std::cout << "waiting for count down latch" << std::endl;
		latch_.wait();
		std::cout << "all threads started" << std::endl;
		for (int i = 0; i < times; ++i) {
			std::string name("hello" + std::to_string(i));
			queue_.put(name);
			std::cout << "thread " << std::this_thread::get_id()
				<< " put data = " << name
				<< ", size = " << queue_.size() << std::endl;
		}
	}

private:
	Test(const Test&) = delete;
	Test& operator=(const Test&) = delete;

	void threadFunc() {
		std::cout << "thread " << std::this_thread::get_id() << " started" << std::endl;
		latch_.countDown();
		bool running = true;
		while (running) {
			std::string d(queue_.take());
			std::cout << "thread " << std::this_thread::get_id()
				<< ", get data = " << d.c_str()
				<< ", size = " << queue_.size() << std::endl;
			running = (d != "stop");
		}
		std::cout << "thread " << std::this_thread::get_id() << " stopped" << std::endl;
	}

	BoundedBlockingQueue<std::string> queue_;
	CountDownLatch latch_;
	std::vector<std::thread> threads_;
};

int main() {
	Test test(5);
	test.run(100);
}