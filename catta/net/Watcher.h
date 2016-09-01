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
	static const int kInvalidActiveIndex = -1;
	typedef std::function<void()> EventCallback;

public:
	Watcher(const int fd, EventLoop* loop);
	~Watcher();
	// fd
	int fd() { return fd_; }
	// callback
	void setCloseCallback(EventCallback&& cb) { closeCallback_ = cb; }
	void setErrorCallback(EventCallback&& cb) { errorCallback_ = cb; }
	void setReadCallback(EventCallback&& cb) { readCallback_ = cb; }
	void setWriteCallback(EventCallback&& cb) { writeCallback_ = cb; }
	// events
	WatcherEvents events() { return events_; }
	void setEvents(WatcherEvents events);
	// revents
	WatcherEvents revents() { return revents_; }
	void containEvents(WatcherEvents revents) { revents_ |= revents; }
	void activeEvents(WatcherEvents revents); // not use in poll
	// active index
	int activeIndex() { return activeIndex_; }
	void setActiveIndex(int activeIndex) { activeIndex_ = activeIndex; }

	void start();
	void stop();

	void handleEvents();

public:
	// noncopyable
	Watcher(const Watcher&) = delete;
	Watcher& operator=(const Watcher&) = delete;

private:
	void update();

	const int fd_;
	EventLoop* loop_;
	// events callback
	EventCallback closeCallback_;
	EventCallback errorCallback_;
	EventCallback readCallback_;
	EventCallback writeCallback_;
	// events
	WatcherEvents events_;
	WatcherEvents revents_;

	bool started_;	// started
	int activeIndex_; // active table index
}; // end class Watcher

} // end namespace catta

#endif // CATTA_NET_WATCHER_H