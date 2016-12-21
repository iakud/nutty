#ifndef CATTA_NET_ACCEPTOR_H
#define CATTA_NET_ACCEPTOR_H

#include <catta/net/Watcher.h>
#include <catta/net/Socket.h>

#include <catta/base/noncopyable.h>

#include <functional>

namespace catta {

class EventLoop;
class InetAddress;

class Acceptor : noncopyable {
public:
	typedef std::function<void(int sockfd, const InetAddress& peerAddr)> ConnectionCallback;

	explicit Acceptor(EventLoop* loop, const InetAddress& localAddr);
	~Acceptor();

	void setConnectionCallback(ConnectionCallback&& cb) {
		connectionCallback_ = cb;
	}

	void start();

private:
	void listen();
	void handleRead();	// read event active

	EventLoop* loop_;
	Socket acceptSocket_;
	Watcher watcher_;
	bool listenning_;	// is listenning
	int idleFd_;

	ConnectionCallback connectionCallback_;
}; // end class Acceptor

} // end namespace catta

#endif // CATTA_NET_ACCEPTOR_H
