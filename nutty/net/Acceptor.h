#ifndef NUTTY_NET_ACCEPTOR_H
#define NUTTY_NET_ACCEPTOR_H

#include <nutty/net/Socket.h>
#include <nutty/base/Watcher.h>

#include <functional>

namespace nutty {

class EventLoop;
class InetAddress;

class Acceptor {
public:
	typedef std::function<void(int sockfd, const InetAddress& peerAddr)> ConnectionCallback;

	explicit Acceptor(EventLoop* loop, const InetAddress& localAddr);
	~Acceptor();

	void setConnectionCallback(ConnectionCallback&& cb) {
		connectionCallback_ = cb;
	}

	void start();

private:
	Acceptor(const Acceptor&) = delete;
	Acceptor& operator=(const Acceptor&) = delete;

	void listen();
	void handleRead();	// read event active

	EventLoop* loop_;
	Socket acceptSocket_;
	Watcher watcher_;
	bool listenning_;	// is listenning
	int idleFd_;

	ConnectionCallback connectionCallback_;
}; // end class Acceptor

} // end namespace nutty

#endif // NUTTY_NET_ACCEPTOR_H
