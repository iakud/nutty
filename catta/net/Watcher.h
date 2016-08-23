#ifndef CATTA_NET_WATCHER_H
#define CATTA_NET_WATCHER_H

namespace catta {

class EventLoop;

enum WatcherEvent {
	none = 1L << 0,
	read = 1L << 1,
	write = 1L << 2,
	error = 1L << 3,
	close = 1L << 4
};

class Watcher {
public:
	typedef std::function<void(WatcherEvent)> EventCallback;

public:
	Watcher(EventLoop* loop, const int fd);
	~Watcher();

	// fd
	int getFd() { return fd_; }
	// events
	void enableRead() { read_ = true; }
	void disableRead() { read_ = false; }
	void enableWrite() { write_ = true; }
	void disableWrite() { write_ = false; }
	void enableAll() { read_ = write_ = true; }
	void disableAll() { read_ = write_ = false; }
	// callback
	void setCloseCallback(EventCallback&& cb) { closeCallback_ = cb; }
	void setErrorCallback(EventCallback&& cb) { errorCallback_ = cb; }
	void setReadCallback(EventCallback&& cb) { readCallback_ = cb; }
	void setWriteCallback(EventCallback&& cb) { writeCallback_ = cb; }

	void setEventCallback(EventCallback&& cb) { eventCallback = cb; }

	void handleEvents();

public:
	// noncopyable
	Watcher(const Watcher&) = delete;
	Watcher& operator=(const Watcher&) = delete;

private:
	const int fd_;
	EventLoop* loop_;
	// events
	bool read_;
	bool write_;
	// revents
	bool rclose_;
	bool rerror_;
	bool rread_;
	bool rwrite_;

	int events_;
	int revents_;
	
	// events callback
	EventCallback closeCallback_;
	EventCallback errorCallback_;
	EventCallback readCallback_;
	EventCallback writeCallback_;

	bool started_;	// started
	bool actived_;	// in active list
	bool readable_;
	bool writeable_;
}; // end class Watcher

} // end namespace catta

#endif // CATTA_NET_WATCHER_H