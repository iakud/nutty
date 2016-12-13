#ifndef CATTA_NET_CONNECTOR_H
#define CATTA_NET_CONNECTOR_H

#include <catta/net/InetAddress.h>
#include <catta/base/noncopyable.h>

#include <memory>
#include <functional>

namespace catta {

class EventLoop;
class Watcher;
class Socket;

class Connector : noncopyable, public std::enable_shared_from_this<Connector> {
public:
	typedef std::function<void(int sockfd, const InetAddress& localAddr)> ConnectionCallback;

	explicit Connector(EventLoop* loop, const InetAddress& peerAddr);
	~Connector();

	void setConnectionCallback(ConnectionCallback&& connectionCallback) {
		connectionCallback_ = connectionCallback;
	}

	void start();
	void restart(); // call in loop
	void stop();

private:
	void connect();
	void connecting();
	void removeAndResetWatcher();
	void resetWatcher();
	void retry();
	void retrying();
	
	void handleError();
	void handleWrite();

	void startInLoop();
	void stopInLoop();

	EventLoop* loop_;
	InetAddress peerAddr_;
	std::unique_ptr<Socket> connectSocket_;
	std::unique_ptr<Watcher> watcher_;
	bool connect_;
	bool connecting_;
	bool retry_;

	ConnectionCallback connectionCallback_;
}; // end class Connector

} // end namespace catta

#endif // CATTA_NET_CONNECTOR_H