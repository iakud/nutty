#ifndef NUTTY_NET_TCPCLIENT_H
#define NUTTY_NET_TCPCLIENT_H

#include <nutty/net/TcpConnection.h>
#include <nutty/net/InetAddress.h>

#include <mutex>
#include <memory>
#include <functional>
#include <atomic>

namespace nutty {

class EventLoop;
class Connector;

class TcpClient {
public:
	TcpClient(EventLoop* loop, const InetAddress& peerAddr);
	~TcpClient();

	bool retry() { return retry_; }
	void enableRetry() { retry_ = true; }

	void setConnectCallback(const ConnectCallback& cb) { connectCallback_ = cb; }
	void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCallback_ = cb; }
	void setReadCallback(const ReadCallback& cb) { readCallback_ = cb; }
	void setWriteCallback(const WriteCallback& cb) { writeCallback_ = cb; }

	void connect();
	void disconnect();

private:
	TcpClient(const TcpClient&) = delete;
	TcpClient& operator=(const TcpClient&) = delete;

	void handleConnection(Socket&& socket, const InetAddress& localAddr);
	void removeConnection(const TcpConnectionPtr& connection);

	EventLoop* loop_;
	InetAddress peerAddr_;
	std::shared_ptr<Connector> connector_;
	std::atomic_bool started_;
	std::atomic_bool retry_;
	std::mutex mutex_;
	TcpConnectionPtr connection_;

	ConnectCallback connectCallback_;
	DisconnectCallback disconnectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
}; // end class TcpClient

} // end namespace nutty

#endif // NUTTY_NET_TCPCLIENT_H