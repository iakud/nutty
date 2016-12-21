#include <nutty/base/Thread.h>

#include <thread>
#include <unistd.h>

using namespace nutty;

void threadFunc() {
	printf("tid=%d\n", CurrentThread::tid());
}

void threadFunc2(int x) {
	printf("tid=%d, x=%d\n", CurrentThread::tid(), x);
}

int main() {
	printf("pid=%d, tid=%d\n", ::getpid(), CurrentThread::tid());
	std::thread t1(threadFunc);
	t1.join();
	std::thread t2(threadFunc2, 77);
	t2.join();
}