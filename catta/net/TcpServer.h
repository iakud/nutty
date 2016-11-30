#ifndef CATTA_NET_TCPSERVER_H
#define CATTA_NET_TCPSERVER_H

#include "InetAddress.h"
#include "TcpConnection.h"

#include <memory>
#include <functional>
#include <map>

namespace catta {

class EventLoop;
class Acceptor;

class TcpServer;
typedef std::shared_ptr<TcpServer> TcpServerPtr;

class TcpServer : noncopyable, public std::enable_shared_from_this<TcpServer> {
public:
	typedef std::function<void(TcpConnectionPtr)> ConnectionCallback;

public:
	explicit TcpServer(EventLoop* loop, const InetAddress& localAddr);
	~TcpServer();

	void setReuseAddr(bool reuseaddr);

	void setEventLoopThreadPool(EventLoopThreadPool* loopThreadPool) {
		loopThreadPool_ = loopThreadPool;
	}

	void setConnectionCallback(ConnectCallback&& cb) {
		connectCallback_ = cb;
	}

	void listen();

private:
	void onAccept(const int sockFd,
			const struct sockaddr_in& peerSockAddr);
	void onClose(const int sockFd, TcpConnectionPtr connection);
	void removeConnection(const int sockFd, TcpConnectionPtr connection);

	EventLoop* loop_;
	InetAddress localAddr_;
	std::shared_ptr<Acceptor> acceptor_;
	bool listen_;
	std::map<int, TcpConnectionPtr> connections_;

	EventLoopThreadPool* loopThreadPool_;
	uint32_t indexLoop_;
	ConnectCallback connectCallback_;
}; // end class TcpServer

} // end namespace net
} // end namespace iak

#endif // IAK_NET_TCPSERVER_H
