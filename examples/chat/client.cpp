#include "codec.h"

#include <nutty/base/EventLoopThread.h>
#include <nutty/net/TcpClient.h>

#include <mutex>
#include <string>
#include <iostream>

using namespace nutty;

class ChatClient {
public:
	ChatClient(EventLoop* loop, const InetAddress& serverAddr)
		: client_(loop, serverAddr)
		, codec_(std::bind(&ChatClient::onMessage, this, std::placeholders::_1, std::placeholders::_2)) {
		client_.setConnectCallback(std::bind(&ChatClient::onConnect, this, std::placeholders::_1));
		client_.setDisconnectCallback(std::bind(&ChatClient::onDisconnect, this, std::placeholders::_1));
		client_.setReadCallback(std::bind(&Codec::onRead, &codec_, std::placeholders::_1, std::placeholders::_2));
		client_.enableRetry();
	}

	void connect() { client_.connect(); }
	void disconnect() { client_.disconnect(); }

	void send(const std::string& message) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (connection_) {
			codec_.send(connection_, message);
		}
	}

private:
	void onConnect(const TcpConnectionPtr& conn) {
		std::unique_lock<std::mutex> lock(mutex_);
		connection_ = conn;
	}

	void onDisconnect(const TcpConnectionPtr& conn) {
		std::unique_lock<std::mutex> lock(mutex_);
		connection_.reset();
	}

	void onMessage(const TcpConnectionPtr&, const std::string& message) {
		printf("<<< %s\n", message.c_str());
	}

	TcpClient client_;
	Codec codec_;
	std::mutex mutex_;
	TcpConnectionPtr connection_;
};


int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <host_ip> <port>\n", argv[0]);
		return 0;
	}

	uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
	InetAddress serverAddr(argv[1], port);

	EventLoopThread loopThread;
	ChatClient client(loopThread.getLoop(), serverAddr);
	client.connect();
	std::string line;
	while (std::getline(std::cin, line)) {
		client.send(line);
	}
	client.disconnect();
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
}