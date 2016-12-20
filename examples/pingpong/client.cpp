#include <catta/net/TcpClient.h>
#include <catta/net/EventLoop.h>
#include <catta/net/InetAddress.h>

#include <string>

#include <iostream>

using namespace catta;

int64_t bytesRead_ = 0;
int64_t bytesWritten_ = 0;
int64_t messagesRead_ = 0;

const std::string message("123123123123123123123123123123123123123123");

void onConnect(const TcpConnectionPtr& conn) {
	//conn->setTcpNoDelay(true);
	std::cout << "connect" << std::endl;
	conn->send(message.data(), static_cast<uint32_t>(message.size()));
}

void onRead(const TcpConnectionPtr& conn, ReceiveBuffer& buffer) {
	int len = buffer.size();
	char data[len];
	buffer.read(data, len);

	++messagesRead_;
    bytesRead_ += len;
    bytesWritten_ += len;

	conn->send(data, len);
}

void onDisconnect(const TcpConnectionPtr& conn) {
	std::cout << "disconnect" << std::endl;
}

int main() {
	EventLoop loop;
	InetAddress serverAddr("127.0.0.1", 8888);
	TcpClient client(&loop, serverAddr);
	client.setConnectCallback(onConnect);
	client.setDisconnectCallback(onDisconnect);
    client.setReadCallback(onRead);
    client.start();
	loop.loop();
}