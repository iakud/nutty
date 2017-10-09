#ifndef NUTTY_EXAMPLES_SIMPLE_ECHO_ECHO_H
#define NUTTY_EXAMPLES_SIMPLE_ECHO_ECHO_H

#include <nutty/net/TcpServer.h>

class EchoServer {
public:
	EchoServer(nutty::EventLoop* loop, const nutty::InetAddress& listenAddr);

	void start();
private:
	void onConnect(const nutty::TcpConnectionPtr& conn);
	void onDisconnect(const nutty::TcpConnectionPtr& conn);
	void onRead(const nutty::TcpConnectionPtr& conn, nutty::ReceiveBuffer& receiveBuffer);

	nutty::TcpServer server_;
}; // end class EchoServer

#endif // NUTTY_EXAMPLES_SIMPLE_ECHO_ECHO_H