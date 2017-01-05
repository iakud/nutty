#include <nutty/base/EventLoop.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <map>
#include <string>
#include <algorithm>

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
	EventLoop loop;
	g_loop = &loop;

	loop.runAfter(std::chrono::seconds(1), std::bind(print, "once 1"));
	loop.runEvery(std::chrono::milliseconds(500), std::bind(print, "every 1"));
	loop.loop();
}