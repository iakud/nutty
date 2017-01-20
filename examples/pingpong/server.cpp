#include <nutty/net/TcpServer.h>
#include <nutty/net/InetAddress.h>
#include <nutty/base/EventLoop.h>

#include <iostream>
#include <string>

using namespace nutty;

void onConnect(const TcpConnectionPtr& conn) {
	conn->setTcpNoDelay(true);
	std::cout << "client connect" << std::endl;
}

void onRead(const TcpConnectionPtr& conn, ReceiveBuffer& buffer) {
	int len = buffer.size();
	char buf[len];
	buffer.read(buf, len);
	std::string message(buf, len);
	conn->send(message.data(), static_cast<uint32_t>(message.size()));
	
	//conn->send(buffer);
}

void onDisconnect(const TcpConnectionPtr& conn) {
	std::cout << "client disconnect" << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc < 4) {
		fprintf(stderr, "Usage: server <address> <port> <threads>\n");
		return 0;
	}
	const char* ip = argv[1];
	uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
	InetAddress listenAddr(ip, port);
	int threadCount = atoi(argv[3]);

	EventLoop loop;
	TcpServer server(&loop, listenAddr);
	server.setConnectCallback(std::bind(onConnect, std::placeholders::_1));
	server.setDisconnectCallback(std::bind(onDisconnect, std::placeholders::_1));
	server.setReadCallback(onRead);
	if (threadCount > 1) {
		server.setThreadNum(threadCount);
	}
	server.start();
	loop.loop();
}