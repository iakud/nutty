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

class TcpServer : noncopyable {
public:
	explicit TcpServer(EventLoop* loop, const InetAddress& localAddr);
	~TcpServer();
/*
	void setEventLoopThreadPool(EventLoopThreadPool* loopThreadPool) {
		loopThreadPool_ = loopThreadPool;
	}
*/
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
	std::atomic_bool listen_;
	std::unordered_map<int, TcpConnectionPtr> connections_;

	//EventLoopThreadPool* loopThreadPool_;
	uint32_t indexLoop_;

	ConnectCallback connectCallback_;
	DisconnectCallback disconnectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
}; // end class TcpServer

} // end namespace catta

#endif // IAK_NET_TCPSERVER_H
