#include <nutty/net/TcpClient.h>

#include <nutty/net/Connector.h>
#include <nutty/base/EventLoop.h>

using namespace nutty;

TcpClient::TcpClient(EventLoop* loop, const InetAddress& peerAddr)
	: loop_(loop)
	, peerAddr_(peerAddr)
	, connector_(std::make_shared<Connector>(loop, peerAddr))
	, started_(false)
	, retry_(false) {
	connector_->setConnectionCallback(std::bind(&TcpClient::handleConnection,
		this, std::placeholders::_1, std::placeholders::_2));
}

TcpClient::~TcpClient() {
	if (connection_) {
		connection_->destroyed();
		connection_.reset();
	}
	if (started_) {
		connector_->stop();
	}
}

void TcpClient::start() {
	bool expected = false;
	if (started_.compare_exchange_strong(expected, true)) {
		connector_->start();
	}
}

void TcpClient::stop() {
	bool expected = true;
	if (started_.compare_exchange_strong(expected, false)) {
		connector_->stop();
	}
}

void TcpClient::handleConnection(int sockfd, const InetAddress& localAddr) {
	TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop_, sockfd, localAddr, peerAddr_);
	connection->setConnectCallback(connectCallback_);
	connection->setDisconnectCallback(disconnectCallback_);
	connection->setReadCallback(readCallback_);
	connection->setWriteCallback(writeCallback_);
	connection->setCloseCallback(std::bind(&TcpClient::removeConnection,
		this, std::placeholders::_1));
	connection_ = connection;
	connection->established();
}

void TcpClient::removeConnection(const TcpConnectionPtr& connection) {
	connection_.reset();
	connection->destroyed();
	if (retry_ && started_) {
		connector_->restart();
	}
}