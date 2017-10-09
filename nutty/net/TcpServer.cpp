#include <nutty/net/TcpServer.h>

#include <nutty/net/Acceptor.h>
#include <nutty/base/EventLoop.h>
#include <nutty/base/EventLoopThreadPool.h>

using namespace nutty;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& localAddr)
	: loop_(loop)
	, localAddr_(localAddr)
	, acceptor_(std::make_unique<Acceptor>(loop, localAddr))
	, threadPool_(std::make_unique<EventLoopThreadPool>(loop))
	, started_(false) {
	acceptor_->setConnectionCallback(std::bind(&TcpServer::handleConnection, 
		this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
	// destroy connections
	for (const TcpConnectionPtr& connection : connections_) {
		// FIXME: unsafe
		connection->destroyed();
	}
	/* FIXME
	if (started_) {
		acceptor_->stop();
	}
	*/
}

void TcpServer::setThreadNum(int numThreads) {
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::listen() {
	bool expected = false;
	if (started_.compare_exchange_strong(expected, true)) {
		threadPool_->start();
		loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
	}
}

void TcpServer::handleConnection(Socket&& socket, const InetAddress& peerAddr) {
	EventLoop* loop = threadPool_->getLoop();
	TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop, std::move(socket), localAddr_, peerAddr);
	connection->setConnectCallback(connectCallback_);
	connection->setDisconnectCallback(disconnectCallback_);
	connection->setReadCallback(readCallback_);
	connection->setWriteCallback(writeCallback_);
	connection->setCloseCallback(std::bind(&TcpServer::removeConnection,
		this, std::placeholders::_1));
	connections_.insert(connection);
	connection->established();
}

void TcpServer::removeConnection(const TcpConnectionPtr& connection) {
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, connection));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& connection) {
	connections_.erase(connection);
	connection->destroyed();
}