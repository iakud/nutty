#ifndef NUTTY_NET_TCPSERVER_H
#define NUTTY_NET_TCPSERVER_H

#include <nutty/net/TcpConnection.h>
#include <nutty/net/InetAddress.h>

#include <memory>
#include <functional>
#include <unordered_map>
#include <atomic>

namespace nutty {

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TcpServer {
public:
	TcpServer(EventLoop* loop, const InetAddress& localAddr);
	~TcpServer();

	void setThreadNum(int numThreads);

	void setConnectCallback(const ConnectCallback&& cb) { connectCallback_ = std::move(cb); }
	void setDisconnectCallback(const DisconnectCallback&& cb) { disconnectCallback_ = std::move(cb); }
	void setReadCallback(const ReadCallback&& cb) { readCallback_ = std::move(cb); }
	void setWriteCallback(const WriteCallback&& cb) { writeCallback_ = std::move(cb); }

	void start();

private:
	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;

	void handleConnection(Socket&& socket, const InetAddress& peerAddr);
	void removeConnection(const int sockfd, const TcpConnectionPtr& connection);
	void removeConnectionInLoop(const int sockfd, const TcpConnectionPtr& connection);

	EventLoop* loop_;
	InetAddress localAddr_;
	std::unique_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> threadPool_;
	std::atomic_bool started_;
	std::unordered_map<int, TcpConnectionPtr> connections_;

	ConnectCallback connectCallback_;
	DisconnectCallback disconnectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
}; // end class TcpServer

} // end namespace nutty

#endif // NUTTY_NET_TCPSERVER_H
