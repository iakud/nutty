#include <catta/net/TcpServer.h>

#include <catta/net/Acceptor.h>
#include <catta/net/EventLoop.h>

#include <catta/net/EventLoopThreadPool.h>

using namespace catta;

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
	for (auto& pairConnnection : connections_) {
		TcpConnectionPtr& connection = pairConnnection.second;
		connection->destroyed();
		connection.reset();
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

void TcpServer::start() {
	bool expected = false;
	if (started_.compare_exchange_strong(expected, true)) {
		threadPool_->start();
		acceptor_->start();
	}
}

void TcpServer::handleConnection(int sockfd, const InetAddress& peerAddr) {
	EventLoop* loop = threadPool_->getLoop();
	TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop, sockfd, localAddr_, peerAddr);
	connection->setConnectCallback(connectCallback_);
	connection->setDisconnectCallback(disconnectCallback_);
	connection->setReadCallback(readCallback_);
	connection->setWriteCallback(writeCallback_);
	connection->setCloseCallback(std::bind(&TcpServer::removeConnection,
		this, sockfd, std::placeholders::_1));
	connections_[sockfd] = connection;
	connection->established();
}

void TcpServer::removeConnection(const int sockfd, const TcpConnectionPtr& connection) {
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,
		this, sockfd, connection));
}

void TcpServer::removeConnectionInLoop(const int sockfd, const TcpConnectionPtr& connection) {
	connections_.erase(sockfd);
	connection->destroyed();
}