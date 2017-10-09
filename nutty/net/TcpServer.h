#ifndef NUTTY_NET_TCPSERVER_H
#define NUTTY_NET_TCPSERVER_H

#include <nutty/net/TcpConnection.h>
#include <nutty/net/InetAddress.h>

#include <memory>
#include <functional>
#include <unordered_set>
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

	void setConnectCallback(const ConnectCallback& cb) { connectCallback_ = cb; }
	void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCallback_ = cb; }
	void setReadCallback(const ReadCallback& cb) { readCallback_ = cb; }
	void setWriteCallback(const WriteCallback& cb) { writeCallback_ = cb; }

	void listen();

private:
	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;

	void handleConnection(Socket&& socket, const InetAddress& peerAddr);
	void removeConnection(const TcpConnectionPtr& connection);
	void removeConnectionInLoop(const TcpConnectionPtr& connection);

	EventLoop* loop_;
	InetAddress localAddr_;
	std::unique_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> threadPool_;
	std::atomic_bool started_;
	std::unordered_set<TcpConnectionPtr> connections_;

	ConnectCallback connectCallback_;
	DisconnectCallback disconnectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
}; // end class TcpServer

} // end namespace nutty

#endif // NUTTY_NET_TCPSERVER_H
