#ifndef CATTA_NET_TCPSERVER_H
#define CATTA_NET_TCPSERVER_H

#include <catta/net/TcpConnection.h>
#include <catta/net/InetAddress.h>

#include <memory>
#include <functional>
#include <unordered_map>
#include <atomic>

namespace catta {

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TcpServer : noncopyable {
public:
	explicit TcpServer(EventLoop* loop, const InetAddress& localAddr);
	~TcpServer();

	void setThreadNum(int numThreads);

	void setConnectCallback(const ConnectCallback& cb) { connectCallback_ = cb; }
	void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCallback_ = cb; }
	void setReadCallback(const ReadCallback& cb) { readCallback_ = cb; }
	void setWriteCallback(const WriteCallback& cb) { writeCallback_ = cb; }

	void listen();

private:
	void handleAccept(const int sockfd, const InetAddress& peerAddr);
	void removeConnection(const int sockfd, TcpConnectionPtr connection);
	void removeConnectionInLoop(const int sockfd, TcpConnectionPtr connection);

	EventLoop* loop_;
	InetAddress localAddr_;
	std::unique_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> threadPool_;
	std::atomic_flag listen_;
	std::unordered_map<int, TcpConnectionPtr> connections_;

	ConnectCallback connectCallback_;
	DisconnectCallback disconnectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
}; // end class TcpServer

} // end namespace catta

#endif // IAK_NET_TCPSERVER_H
