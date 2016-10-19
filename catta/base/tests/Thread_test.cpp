#include <catta/base/Thread.h>

#include <thread>
#include <unistd.h>

void threadFunc() {
	printf("tid=%d\n", catta::CurrentThread::tid());
}

void threadFunc2(int x) {
	printf("tid=%d, x=%d\n", catta::CurrentThread::tid(), x);
}

int main() {
	printf("pid=%d, tid=%d\n", ::getpid(), catta::CurrentThread::tid());
	std::thread t1(threadFunc);
	t1.join();
	std::thread t2(threadFunc2, 77);
	t2.join();
}