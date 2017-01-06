#include <nutty/base/EventLoop.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <map>
#include <string>
#include <algorithm>
#include <functional>

using namespace nutty;

int cnt = 0;
EventLoop* g_loop;

void print(const char* msg) {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::cout << std::put_time(std::localtime(&now), "%F %T ") << msg << std::endl;
	if (++cnt == 20) {
		g_loop->quit();
	}
}

TimerPtr timerCancel;

void cancel(const char* msg) {
	print(msg);
	if (timerCancel) {
		g_loop->cancel(timerCancel);
		timerCancel.reset();
	}
}

int main() {
	EventLoop loop;
	g_loop = &loop;

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	loop.runAt(now, std::bind(print, "start timer"));
	loop.runAt(now + std::chrono::seconds(3), std::bind(print, "timer 1"));
	TimerPtr timer2 = loop.runAt(now + std::chrono::seconds(1), std::bind(print, "timer 2"));
	loop.cancel(timer2);

	loop.runAfter(std::chrono::milliseconds(2500), std::bind(print, "timer 3"));
	TimerPtr timer4 = loop.runAfter(std::chrono::milliseconds(50), std::bind(print, "timer 4"));
	loop.cancel(timer4);

	loop.runEvery(std::chrono::milliseconds(1000), std::bind(print, "timer 5"));
	TimerPtr timer6 = loop.runEvery(std::chrono::milliseconds(800), std::bind(cancel, "timer 6"));
	timerCancel = timer6;
	TimerPtr timer7 = loop.runEvery(std::chrono::milliseconds(100), std::bind(print, "timer 7"));
	loop.cancel(timer7);
	loop.loop();
}