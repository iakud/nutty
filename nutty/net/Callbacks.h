#ifndef NUTTY_NET_CALLBACKS_H
#define NUTTY_NET_CALLBACKS_H

#include <functional>
#include <memory>

namespace nutty {

class ReceiveBuffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

// callback typedef
typedef std::function<void(const TcpConnectionPtr&)> ConnectCallback;
typedef std::function<void(const TcpConnectionPtr&, ReceiveBuffer&)> ReadCallback;
typedef std::function<void(const TcpConnectionPtr&, uint32_t)> WriteCallback;
typedef std::function<void(const TcpConnectionPtr&)> DisconnectCallback;

void defaultConnectCallback(const TcpConnectionPtr&);
void defaultReadCallback(const TcpConnectionPtr&, ReceiveBuffer&);

} // end namespace nutty

#endif // NUTTY_NET_CALLBACKS_H
