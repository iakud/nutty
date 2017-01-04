#include <nutty/base/EventLoop.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <time.h>

using namespace nutty;

int cnt = 0;
EventLoop* g_loop;

void print(const char* msg) {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::cout << "msg " << std::put_time(std::localtime(&now), "%F %T") << " " << msg << std::endl;
	if (++cnt == 20) {
		g_loop->quit();
	}
}

int main() {
	// FIXME : test
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::nanoseconds epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
	std::cout << "timestamp sec: " << epoch.count() * std::chrono::nanoseconds::period::num / std::chrono::nanoseconds::period::den << std::endl;

	std::cout << "timestamp nsec: " << epoch.count() * std::chrono::nanoseconds::period::num % std::chrono::nanoseconds::period::den << std::endl;

	struct timespec ts;
	::clock_gettime(CLOCK_MONOTONIC,&ts);
	std::cout << ts.tv_sec << " " << ts.tv_nsec << std::endl;


	EventLoop loop;
	g_loop = &loop;

	loop.runAfter(std::chrono::seconds(1), std::bind(print, "once 1"));
	loop.loop();
}