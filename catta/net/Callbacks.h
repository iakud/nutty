#ifndef CATTA_NET_CALLBACKS_H
#define CATTA_NET_CALLBACKS_H

#include <functional>
#include <memory>

namespace catta {

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

} // end namespace catta

#endif // CATTA_NET_CALLBACKS_H
