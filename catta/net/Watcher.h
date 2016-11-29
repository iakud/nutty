#ifndef CATTA_NET_WATCHER_H
#define CATTA_NET_WATCHER_H

#include <catta/base/noncopyable.h>

#include <functional>

namespace catta {

class EventLoop;

class Watcher : noncopyable {
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
	void containEvents(int revents) { revents_ = revents; }
	void handleEvents();

	void start();
	void stop();

private:
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

} // end namespace catta

#endif // CATTA_NET_WATCHER_H