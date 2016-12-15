#ifndef CATTA_NET_TCPCONNECTION_H
#define CATTA_NET_TCPCONNECTION_H

#include <catta/net/Callbacks.h>
#include <catta/net/Buffer.h>
#include <catta/net/InetAddress.h>

#include <catta/base/noncopyable.h>

#include <memory>
#include <functional>
#include <string>

namespace catta {

class EventLoop;
class Watcher;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
	explicit TcpConnection(EventLoop* loop, int sockfd,
		const InetAddress& localAddr, const InetAddress& peerAddr);
	~TcpConnection();
	
	EventLoop* getLoop() const { return loop_; }
	const InetAddress& localAddress() const { return localAddr_; }
	const InetAddress& peerAddress() const { return peerAddr_; }

	void setConnectCallback(const ConnectCallback& cb) { connectCallback_ = cb; }
	void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCallback_ = cb; }
	void setReadCallback(const ReadCallback& cb) { readCallback_ = cb; }
	void setWriteCallback(const WriteCallback& cb) { writeCallback_ = cb; }

	void send(const void* buf, uint32_t count);
	void shutdown();
	void forceClose();

private:
	typedef std::shared_ptr<Buffer> BufferPtr;
	enum State { kDisconnected, kConnecting, kConnected, kDisconnecting };

	// for tcpserver and tcpclient
	typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
	void setCloseCallback(CloseCallback&& cb) { closeCallback_ = cb; }

	void established();
	void destroyed();

	void handleClose();
	void handleError();
	void handleRead();
	void handleWrite();

	void sendInLoop(const void* buf, uint32_t count);
	void sendInLoop(BufferPtr& buffer);
	void shutdownInLoop();
	void forceCloseInLoop();
	void establishedInLoop();
	void destroyedInLoop();

	void setState(State state) { state_ = state; }

private:
	EventLoop* loop_;
	State state_;
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Watcher> watcher_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;
	bool writable_;

	SendBuffer sendBuffer_;
	ReceiveBuffer receiveBuffer_;

	ConnectCallback connectCallback_;
	DisconnectCallback disconnectCallback_;
	ReadCallback readCallback_;
	WriteCallback writeCallback_;
	CloseCallback closeCallback_;

	// friend class only TcpServer & TcpClient
	friend class TcpServer;
	friend class TcpClient;
}; // end class TcpConnection

} // end namespace catta

#endif // CATTA_NET_TCPCONNECTION_H