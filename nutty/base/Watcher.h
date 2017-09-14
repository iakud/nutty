#ifndef NUTTY_BASE_WATCHER_H
#define NUTTY_BASE_WATCHER_H

#include <functional>

namespace nutty {

class EventLoop;

class Watcher {
public:
	typedef std::function<void()> EventCallback;

	Watcher(EventLoop* loop, const int fd);
	~Watcher();

	// callback
	void setCloseCallback(EventCallback&& cb) { closeCallback_ = cb; }
	void setErrorCallback(EventCallback&& cb) { errorCallback_ = cb; }
	void setReadCallback(EventCallback&& cb) { readCallback_ = cb; }
	void setWriteCallback(EventCallback&& cb) { writeCallback_ = cb; }
	// fd
	int fd() { return fd_; }
	// events
	int events() const { return events_; }
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
	bool isReading() const { return events_ & kReadEvent; }
	bool isWriting() const { return events_ & kWriteEvent; }
	// revents
	void revents(int revents) { revents_ = revents; }
	void handleEvents();

private:
	Watcher(const Watcher&) = delete;
	Watcher& operator=(const Watcher&) = delete;

	void update();

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	EventLoop* loop_;
	const int fd_;
	// events
	int events_;
	int revents_;
	bool started_;	// started
	// events callback
	EventCallback closeCallback_;
	EventCallback errorCallback_;
	EventCallback readCallback_;
	EventCallback writeCallback_;
}; // end class Watcher

} // end namespace nutty

#endif // NUTTY_BASE_WATCHER_H