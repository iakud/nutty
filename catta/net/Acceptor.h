#ifndef CATTA_NET_ACCEPTOR_H
#define CATTA_NET_ACCEPTOR_H

#include <functional>

#include <arpa/inet.h>

namespace catta {

class EventLoop;
class Watcher;

class Acceptor {

public:
	typedef std::function<void(int sockFd,
			const struct sockaddr_in& remoteSockAddr)> AcceptCallback;

public:
	explicit Acceptor(EventLoop* loop,
			const struct sockaddr_in& localSockAddr);
	~Acceptor();

	void setAcceptCallback(AcceptCallback&& acceptCallback) {
		acceptCallback_ = acceptCallback;
	}

	bool isListenning() const { return listenning_; }
	void listen();

public:
	// noncopyable
	Acceptor(const Acceptor&) = delete;
	Acceptor& operator=(const Acceptor&) = delete;

private:
	void handleRead();	// read event active

	EventLoop* loop_;
	const struct sockaddr_in localSockAddr_;
	const int sockFd_;
	Watcher watcher_;
	bool listenning_;	// is listenning
	int idleFd_;

	AcceptCallback acceptCallback_;
}; // end class Acceptor

} // end namespace catta

#endif // CATTA_NET_ACCEPTOR_H
