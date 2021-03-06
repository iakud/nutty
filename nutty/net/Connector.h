#ifndef NUTTY_NET_CONNECTOR_H
#define NUTTY_NET_CONNECTOR_H

#include <nutty/net/InetAddress.h>

#include <memory>
#include <functional>

namespace nutty {

class EventLoop;
class Watcher;
class Socket;

class Connector : public std::enable_shared_from_this<Connector> {
public:
	typedef std::function<void(Socket&& socket, const InetAddress& localAddr)> ConnectionCallback;

	explicit Connector(EventLoop* loop, const InetAddress& peerAddr);
	~Connector();

	void setConnectionCallback(ConnectionCallback&& cb) { connectionCallback_ = std::move(cb); }

	void start();
	void restart(); // call in loop
	void stop();

private:
	static const int kMaxRetryDelayMs = 30 * 1000;
	static const int kInitRetryDelayMs = 500;

	Connector(const Connector&) = delete;
	Connector& operator=(const Connector&) = delete;
	
	void connect();
	void connecting();
	void stopAndResetWatcher();
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
	int retryDelayMs_;

	ConnectionCallback connectionCallback_;
}; // end class Connector

} // end namespace nutty

#endif // NUTTY_NET_CONNECTOR_H