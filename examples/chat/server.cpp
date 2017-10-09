#include "codec.h"

#include <nutty/base/EventLoop.h>
#include <nutty/net/TcpServer.h>

#include <unordered_set>
#include <string>

using namespace nutty;

class ChatServer {
public:
	ChatServer(EventLoop* loop, const InetAddress& listenAddr)
		: server_(loop, listenAddr)
		, codec_(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2)) {
		server_.setConnectCallback(std::bind(&ChatServer::onConnect, this, std::placeholders::_1));
		server_.setDisconnectCallback(std::bind(&ChatServer::onDisconnect, this, std::placeholders::_1));
		server_.setReadCallback(std::bind(&Codec::onRead, &codec_, std::placeholders::_1, std::placeholders::_2));
	}

	void listen() { server_.listen(); }

private:
	void onConnect(const TcpConnectionPtr& conn) {
		connections_.insert(conn);
	}

	void onDisconnect(const TcpConnectionPtr& conn) {
		connections_.erase(conn);
	}

	void onMessage(const TcpConnectionPtr&, const std::string& message) {
		for (TcpConnectionPtr conn : connections_) {
			codec_.send(conn, message);
		}
	}

	TcpServer server_;
	Codec codec_;
	std::unordered_set<TcpConnectionPtr> connections_;
};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <address> <port>\n", argv[0]);
		return 0;
	}

	uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
	InetAddress serverAddr(argv[1], port);

	EventLoop loop;
	ChatServer server(&loop, serverAddr);
	server.listen();
	loop.loop();
}