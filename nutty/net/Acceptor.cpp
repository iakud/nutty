#include <nutty/net/Acceptor.h>

#include <nutty/net/InetAddress.h>
#include <nutty/base/EventLoop.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

using namespace nutty;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& localAddr)
	: loop_(loop)
	, acceptSocket_()
	, acceptWatcher_(loop, acceptSocket_.fd())
	, listenning_(false)
	, idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.bind(localAddr.getSockAddr());
	acceptWatcher_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
	acceptWatcher_.disableAll();
	::close(idleFd_);
}

void Acceptor::listen() {
	if (listenning_) return;
	listenning_ = true;
	acceptSocket_.listen();
	acceptWatcher_.enableReading();
}

void Acceptor::handleRead() {
	struct sockaddr_in peerSockAddr;
	Socket socket(acceptSocket_.accept(peerSockAddr));
	if (socket.fd() < 0) {
		int err = errno; // on error
		if (EAGAIN == err) {

		} else if (EMFILE == err || ENFILE == err) {
			::close(idleFd_);
			acceptSocket_.accept();
			idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		} else {
			// FIXME
			// LOG_FATAL
		}
	} else { // accept successful
		InetAddress peerAddr(peerSockAddr);
		if (connectionCallback_) {
			connectionCallback_(std::move(socket), peerAddr);
		}
	}
}