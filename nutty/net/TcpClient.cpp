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
	TcpConnectionPtr connection;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		connection = connection_;
	}
	// FIXME: unsafe
	if (connection) {
		connection->destroyed();
		connection.reset();
	}
	if (started_) {
		connector_->stop();
	}
}

void TcpClient::connect() {
	bool expected = false;
	if (started_.compare_exchange_strong(expected, true)) {
		connector_->start();
	}
}

void TcpClient::disconnect() {
	bool expected = true;
	if (started_.compare_exchange_strong(expected, false)) {
		connector_->stop();
	}
	std::unique_lock<std::mutex> lock(mutex_);
	if (connection_) {
		connection_->shutdown();
	}
}

void TcpClient::handleConnection(Socket&& socket, const InetAddress& localAddr) {
	TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop_, std::move(socket), localAddr, peerAddr_);
	connection->setConnectCallback(connectCallback_);
	connection->setDisconnectCallback(disconnectCallback_);
	connection->setReadCallback(readCallback_);
	connection->setWriteCallback(writeCallback_);
	connection->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
	{
		std::unique_lock<std::mutex> lock(mutex_);
		connection_ = connection;
	}
	connection->established();
}

void TcpClient::removeConnection(const TcpConnectionPtr& connection) {
	{
		std::unique_lock<std::mutex> lock(mutex_);
		connection_.reset();
	}
	connection->destroyed();
	if (retry_ && started_) {
		connector_->restart();
	}
}