#include <catta/net/TcpServer.h>

#include <catta/net/Acceptor.h>
#include <catta/net/EventLoop.h>

using namespace catta;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& localAddr)
	: loop_(loop)
	, localAddr_(localAddr)
	, acceptor_(std::make_unique<Acceptor>(loop, localAddr))
	, listen_(false)
//	, loopThreadPool_(NULL)
	, indexLoop_(0) {
	acceptor_->setAcceptCallback(std::bind(&TcpServer::handleAccept, 
		this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
	// destroy connections
	for (auto& pairConnnection : connections_) {
		TcpConnectionPtr& connection = pairConnnection.second;
		connection->destroyed();
		connection.reset();
	}
	/*
	if (loopThreadPool_) {
		loopThreadPool_ = NULL;
	}*/
}

void TcpServer::listen() {
	if (listen_) {
		return;
	}

	listen_ = true;
	acceptor_->listen();
}

void TcpServer::handleAccept(const int sockfd, const InetAddress& peerAddr) {
	/*
	EventLoop* loop;
	if (loopThreadPool_) {
		++indexLoop_;
		indexLoop_ %= loopThreadPool_->getCount();
		loop = loopThreadPool_->getLoop(indexLoop_);
	} else {
		loop = loop_;
	}
	*/
	EventLoop* loop = loop_; // FIXME

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