#ifndef CATTA_NET_CONNECTOR_H
#define CATTA_NET_CONNECTOR_H

#include <catta/net/InetAddress.h>

#include <memory>
#include <functional>

namespace catta {

class EventLoop;
class Watcher;
class Socket;

class Connector {
public:
	typedef std::function<void(int sockfd, const InetAddress& localAddr)> ConnectionCallback;

	explicit Connector(EventLoop* loop, const InetAddress& peerAddr);
	~Connector();

	void setConnectionCallback(ConnectionCallback&& connectionCallback) {
		connectionCallback_ = connectionCallback;
	}

	void connect();
	void reconnect();
	void disconnect();

private:
	void retry();
	void resetWatcher();
	
	void handleWrite();
	void handleError();

	void connectInLoop();
	void disconnectInLoop();

	EventLoop* loop_;
	InetAddress peerAddr_;
	std::unique_ptr<Socket> connectSocket_;
	std::unique_ptr<Watcher> watcher_;
	bool connect_;
	bool connecting_;

	ConnectionCallback connectionCallback_;
}; // end class Connector

} // end namespace catta

#endif // CATTA_NET_CONNECTOR_H