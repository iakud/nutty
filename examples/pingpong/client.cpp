#include <nutty/base/EventLoop.h>
#include <nutty/base/EventLoopThreadPool.h>
#include <nutty/net/TcpClient.h>
#include <nutty/net/InetAddress.h>

#include <string>
#include <iostream>

using namespace nutty;

class Client;

class Session {
public:
	Session(Client* owner, EventLoop* loop, const InetAddress& serverAddr)
		: owner_(owner)
		, client_(loop, serverAddr)
		, bytesRead_(0)
		, bytesWritten_(0)
		, countRead_(0) {
		client_.setConnectCallback(std::bind(&Session::onConnect, this, std::placeholders::_1));
		client_.setDisconnectCallback(std::bind(&Session::onDisconnect, this, std::placeholders::_1));
		client_.setReadCallback(std::bind(&Session::onRead, this, std::placeholders::_1, std::placeholders::_2));
	}

	void start() { client_.start(); }
	void stop() { if (conn_) conn_->shutdown(); }

	int64_t bytesRead() const { return bytesRead_; }
	int64_t countRead() const { return countRead_; }
private:
	void onConnect(const TcpConnectionPtr& conn);
	void onDisconnect(const TcpConnectionPtr& conn);
	void onRead(const TcpConnectionPtr& conn, ReceiveBuffer& buffer);

	Client* owner_;
	TcpClient client_;
	TcpConnectionPtr conn_;
	uint64_t bytesRead_;
	uint64_t bytesWritten_;
	uint64_t countRead_;
};

class Client {
public:
	Client(EventLoop* loop, const InetAddress& serverAddr, int blockSize,
		int sessionCount, int timeout, int threadCount)
		: loop_(loop)
		, threadPool_(loop)
		, sessionCount_(sessionCount)
		, timeout_(timeout)
		, numConnected_(0) {
		for (int i = 0; i < blockSize; ++i) {
			message_.push_back(static_cast<char>(i % 128));
		}

		if (threadCount > 1) {
			threadPool_.setThreadNum(threadCount);
		}
		threadPool_.start();

		for (int i = 0; i < sessionCount; ++i) {
			Session* session = new Session(this, threadPool_.getLoop(), serverAddr);
			session->start();
			sessions_.push_back(session);
		}

		loop->runAfter(std::chrono::seconds(timeout), std::bind(&Client::handleTimeout, this));
	}

	~Client() {
		for (Session*& session : sessions_) {
			delete session;
		}
	}

	const std::string& message() const { return message_; }

	void onConnect() {
		if (++numConnected_ == sessionCount_) {
			std::cout << "all connected" << std::endl;
		}
	}

	void onDisconnect() {
		if (--numConnected_ == 0) {
			std::cout << "all disconnected" << std::endl;

			int64_t totalBytesRead = 0;
			int64_t totalCountRead = 0;
			for (Session*& session : sessions_) {
				totalBytesRead += session->bytesRead();
				totalCountRead += session->countRead();
			}
			std::cout << totalBytesRead << " total bytes read" << std::endl;
			std::cout << totalCountRead << " total count read" << std::endl;
			std::cout << static_cast<double>(totalBytesRead) / static_cast<double>(totalCountRead) << " average message size" << std::endl;
			std::cout << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024) << " MiB/s throughput" << std::endl;
			loop_->queueInLoop(std::bind(&Client::quit, this));
		}
	}

private:
	void quit() {
		loop_->queueInLoop(std::bind(&EventLoop::quit, loop_));
	}

	void handleTimeout() {
		for (Session*& session : sessions_) {
			session->stop();
		}
	}

	EventLoop* loop_;
	EventLoopThreadPool threadPool_;
	int sessionCount_;
	int timeout_;
	std::vector<Session*> sessions_;
	std::string message_;
	std::atomic<int> numConnected_;
};

void Session::onConnect(const TcpConnectionPtr& conn) {
	conn->setTcpNoDelay(true);
	conn_ = conn;
	conn->send(owner_->message());
	owner_->onConnect();
}

void Session::onDisconnect(const TcpConnectionPtr& conn) {
	owner_->onDisconnect();
	conn_.reset();
}

void Session::onRead(const TcpConnectionPtr& conn, ReceiveBuffer& buffer) {
	++countRead_;
	bytesRead_ += buffer.size();
	bytesWritten_ += buffer.size();

	conn->send(buffer);
	buffer.retrieveAll();
}

int main(int argc, char* argv[]) {
	if (argc != 7) {
		fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> <sessions> <time>\n");
		return 0;
	}
	const char* ip = argv[1];
	uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
	int threadCount = atoi(argv[3]);
	int blockSize = atoi(argv[4]);
	int sessionCount = atoi(argv[5]);
	int timeout = atoi(argv[6]);

	EventLoop loop;
	InetAddress serverAddr(ip, port);
	Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
	loop.loop();
}