#include "echo.h"

using namespace nutty;

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAddr)
	: server_(loop, listenAddr) {
	server_.setConnectCallback(std::bind(&EchoServer::onConnect, this, std::placeholders::_1));
	server_.setDisconnectCallback(std::bind(&EchoServer::onDisconnect, this, std::placeholders::_1));
	server_.setReadCallback(std::bind(&EchoServer::onRead, this, std::placeholders::_1, std::placeholders::_2));
}

void EchoServer::EchoServer::start() {
	server_.listen();
}

void EchoServer::onConnect(const TcpConnectionPtr& conn) {
}

void EchoServer::onDisconnect(const TcpConnectionPtr& conn) {
}

void EchoServer::onRead(const TcpConnectionPtr& conn, ReceiveBuffer& receiveBuffer) {
	conn->send(receiveBuffer);
}