#include "echo.h"

#include <nutty/base/EventLoop.h>

using namespace nutty;

int main() {
	EventLoop loop;
	InetAddress listenAddr("127.0.0.1", 2007);
	EchoServer server(&loop, listenAddr);
	server.start();
	loop.loop();
}