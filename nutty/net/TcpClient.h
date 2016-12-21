#ifndef CATTA_NET_TCPCLIENT_H
#define CATTA_NET_TCPCLIENT_H

#include <catta/net/TcpConnection.h>
#include <catta/net/InetAddress.h>
#include <catta/base/noncopyable.h>

#include <memory>
#include <functional>
#include <atomic>

namespace catta {

class EventLoop;
class Connector;

class TcpClient : noncopyable {
public:
	TcpClient(EventLoop* loop, const InetAddress& peerAddr);
	~TcpClient();

	bool retry() { return retry_; }
	void enableRetry() { retry_ = true; }

	void setConnectCallback(const ConnectCallback& cb) { connectCallback_ = cb; }
	void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCallback_ = cb; }
	void setReadCallback(const ReadCallback& cb) { readCallback_ = cb; }
	void setWriteCallback(const WriteCallback& cb) { writeCallback_ = cb; }

	void start();
	void stop();

private:
	void handleConnection(int sockfd, const InetAddress& localAddr);
	void removeConnection(const TcpConnectionPtr& connection);

	EventLoop* loop_;
	InetAddress peerAddr_;
	std::shared_ptr<Connector> connector_;
	std::atomic_bool started_;
	std::atomic_bool retry_;
	TcpConnectionPtr connection_;

	ConnectCallback connectCallback_;
	DisconnectCallback disconnectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
}; // end class TcpClient

} // end namespace catta

#endif // CATTA_NET_TCPCLIENT_H