#include <catta/net/Acceptor.h>

#include <catta/net/InetAddress.h>
#include <catta/net/EventLoop.h>
#include <catta/net/Watcher.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace catta;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& localAddr)
: loop_(loop)
, acceptSocket_(Socket::create())
, watcher_(loop, acceptSocket_.fd())
, listenning_(false)
, idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.bind(localAddr.getSockAddr());

	watcher_.setReadCallback(std::bind(&Acceptor::handleRead, this));
	watcher_.setEvents(WatcherEvents::kEventRead);
}

Acceptor::~Acceptor() {
	::close(idleFd_);
	watcher_.stop();
}

void Acceptor::listen() {
	listenning_ = true;
	acceptSocket_.listen();
	watcher_.start();
}

void Acceptor::handleRead() {
	InetAddress peerAddr;
	struct sockaddr_in peerSockAddr;
	int connfd = acceptSocket_.accept(&peerSockAddr);
	if (connfd >= 0) { // accept successful
		peerAddr.setSockAddr(peerSockAddr);
		if (acceptCallback_) {
			acceptCallback_(connfd, peerAddr);
		} else {
			Socket::close(connfd);
		}

		watcher_.activeEvents(WatcherEvents::kEventRead);
	} else {
		int err = errno; // on error
		if (EAGAIN == err) {

		} else if (EMFILE == err || ENFILE == err) {
			::close(idleFd_);
			idleFd_ = acceptSocket_.accept(nullptr);
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);

			watcher_.activeEvents(WatcherEvents::kEventRead);
		} else {
			// FIXME
			// LOG_FATAL
		}
	}
}