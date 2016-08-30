#ifndef CATTA_NET_WATCHER_H
#define CATTA_NET_WATCHER_H

#include <functional>

namespace catta {

enum WatcherEvents {
	kEventNone	= 0,
	kEventRead	= 1L << 0,
	kEventWrite	= 1L << 1,
	kEventError	= 1L << 2,
	kEventClose	= 1L << 3
};

inline WatcherEvents operator&(WatcherEvents a, WatcherEvents b) {
	return WatcherEvents(static_cast<int>(a) & static_cast<int>(b));
}

inline WatcherEvents operator|(WatcherEvents a, WatcherEvents b) {
	return WatcherEvents(static_cast<int>(a) | static_cast<int>(b));
}

inline WatcherEvents operator^(WatcherEvents a, WatcherEvents b) {
	return WatcherEvents(static_cast<int>(a) ^ static_cast<int>(b));
}

inline WatcherEvents operator~(WatcherEvents a) {
	return WatcherEvents(~static_cast<int>(a));
}

inline WatcherEvents operator&=(WatcherEvents a, WatcherEvents b) {
	return a = a & b;
}

inline WatcherEvents operator|=(WatcherEvents a, WatcherEvents b) {
	return a = a | b;
}

inline WatcherEvents operator^=(WatcherEvents a, WatcherEvents b) {
	return a = a ^ b;
}

class EventLoop;

class Watcher {
public:
	typedef std::function<bool()> EventCallback;

public:
	Watcher(const int fd, EventLoop* loop);
	~Watcher();

	// fd
	int fd() { return fd_; }
	// events
	WatcherEvents events() { return events_; }
	void enableReading() { events_ |= kEventRead; update(); }
	void disableReading() { events_ &= ~kEventRead; update(); }
	void enableWriting() { events_ |= kEventWrite; update(); }
	void disableWriting() { events_ &= ~kEventWrite; update(); }
	void enableAll() { events_ = kEventRead | kEventWrite; update(); }
	void disableAll() { events_ = kEventNone; update(); }
	// revents
	WatcherEvents revents() { return revents_; }
	void containEvents(WatcherEvents revents) { revents_ |= revents; }
	// callback
	void setCloseCallback(EventCallback&& cb) { closeCallback_ = cb; }
	void setErrorCallback(EventCallback&& cb) { errorCallback_ = cb; }
	void setReadCallback(EventCallback&& cb) { readCallback_ = cb; }
	void setWriteCallback(EventCallback&& cb) { writeCallback_ = cb; }

	void start();
	void update();
	void stop();

	void handleEvents();

public:
	// noncopyable
	Watcher(const Watcher&) = delete;
	Watcher& operator=(const Watcher&) = delete;

private:
	const int fd_;
	EventLoop* loop_;
	// events
	WatcherEvents events_;
	WatcherEvents revents_;
	// events callback
	EventCallback closeCallback_;
	EventCallback errorCallback_;
	EventCallback readCallback_;
	EventCallback writeCallback_;

	bool started_;	// started
}; // end class Watcher

} // end namespace catta

#endif // CATTA_NET_WATCHER_H