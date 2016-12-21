#include <nutty/net/Acceptor.h>

#include <nutty/net/InetAddress.h>
#include <nutty/base/EventLoop.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace nutty;

Acceptor::Acceptor(EventLoop* loop,const InetAddress& localAddr)
	: loop_(loop)
	, acceptSocket_(Socket::create())
	, watcher_(loop, acceptSocket_.fd())
	, listenning_(false)
	, idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.bind(localAddr.getSockAddr());

	watcher_.setReadCallback(std::bind(&Acceptor::handleRead, this));
	watcher_.enableReading();
}

Acceptor::~Acceptor() {
	::close(idleFd_);
	watcher_.stop();
}

void Acceptor::start() {
	loop_->runInLoop(std::bind(&Acceptor::listen, this));
}

void Acceptor::listen() {
	if (!listenning_) {
		listenning_ = true;
		acceptSocket_.listen();
		watcher_.start();
	}
}

void Acceptor::handleRead() {
	InetAddress peerAddr;
	struct sockaddr_in peerSockAddr;
	int sockfd = acceptSocket_.accept(peerSockAddr);
	if (sockfd >= 0) { // accept successful
		peerAddr.setSockAddr(peerSockAddr);
		if (connectionCallback_) {
			connectionCallback_(sockfd, peerAddr);
		} else {
			Socket::close(sockfd); //Socket::close(sockfd);
		}
	} else {
		int err = errno; // on error
		if (EAGAIN == err) {

		} else if (EMFILE == err || ENFILE == err) {
			::close(idleFd_);
			idleFd_ = acceptSocket_.accept();
			::close(idleFd_);
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		} else {
			// FIXME
			// LOG_FATAL
		}
	}
}