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
	, listen_(ATOMIC_FLAG_INIT) {
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
}

void TcpServer::setThreadNum(int numThreads) {
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::listen() {
	if (!listen_.test_and_set()) {
		threadPool_->start();
		loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
	}
}

void TcpServer::handleConnection(int sockfd, const InetAddress& peerAddr) {
	EventLoop* loop = threadPool_->getLoop();
	TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop, sockfd, localAddr_, peerAddr);
	connections_[sockfd] = connection;
	connection->setConnectCallback(connectCallback_);
	connection->setDisconnectCallback(disconnectCallback_);
	connection->setReadCallback(readCallback_);
	connection->setWriteCallback(writeCallback_);
	connection->setCloseCallback(std::bind(&TcpServer::removeConnection,
		this, sockfd, std::placeholders::_1));
	connection->established();
}

void TcpServer::removeConnection(const int sockfd, TcpConnectionPtr connection) {
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,
		this, sockfd, connection));
}

void TcpServer::removeConnectionInLoop(const int sockfd, TcpConnectionPtr connection) {
	connections_.erase(sockfd);
	connection->destroyed();
}