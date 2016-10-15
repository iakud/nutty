#ifndef CATTA_NET_TCPCONNECTION_H
#define CATTA_NET_TCPCONNECTION_H

#include <catta/net/InetAddress.h>

#include <catta/util/noncopyable.h>

#include <memory>
#include <functional>

namespace catta {

class EventLoop;
class Watcher;
class Socket;

class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
	// callback typedef
	typedef std::function<void(TcpConnectionPtr)> ConnectCallback;
	typedef std::function<void(TcpConnectionPtr, uint32_t)> ReadCallback;
	typedef std::function<void(TcpConnectionPtr, uint32_t)> WriteCallback;
	typedef std::function<void(TcpConnectionPtr)> DisconnectCallback;

private:
	// for tcpserver and tcpclient
	typedef std::function<void(TcpConnectionPtr)> CloseCallback;

	explicit TcpConnection(EventLoop* loop,
		int sockfd,
		const InetAddress& localAddr,
		const InetAddress& peerAddr);
	~TcpConnection();

private:
	EventLoop* loop_;
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Watcher> watcher_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;

	ConnectCallback connectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
	DisconnectCallback disconnectCallback_;
	CloseCallback closeCallback_;

	// friend class only TcpServer & TcpClient
	friend class TcpServer;
	friend class TcpClient;
}; // end class TcpConnection

} // end namespace catta

#endif // CATTA_NET_TCPCONNECTION_H