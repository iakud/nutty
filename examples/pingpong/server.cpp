#include <nutty/net/TcpServer.h>
#include <nutty/net/InetAddress.h>
#include <nutty/base/EventLoop.h>

#include <iostream>
#include <string>

using namespace nutty;

void onConnect(const TcpConnectionPtr& conn) {
	//conn->setTcpNoDelay(true);
	std::cout << "client connect" << std::endl;
}

void onRead(const TcpConnectionPtr& conn, ReceiveBuffer& buffer) {
	int len = buffer.size();
	char data[len];
	buffer.read(data, len);
	std::string message(data, len);
	std::cout << message << std::endl;
	conn->send(message.data(), static_cast<uint32_t>(message.size()));
}

void onDisconnect(const TcpConnectionPtr& conn) {
	std::cout << "client disconnect" << std::endl;
}

int main() {
	EventLoop loop;
	InetAddress listenAddr("127.0.0.1", 8888);
	TcpServer server(&loop, listenAddr);
	server.setConnectCallback(std::bind(onConnect, std::placeholders::_1));
	server.setDisconnectCallback(std::bind(onDisconnect, std::placeholders::_1));
	server.setReadCallback(onRead);
	server.start();
	loop.loop();
}