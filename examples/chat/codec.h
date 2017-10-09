#ifndef NUTTY_EXAMPLES_CHAT_CODEC_H
#define NUTTY_EXAMPLES_CHAT_CODEC_H

#include <nutty/net/TcpConnection.h>

#include <string>

#include <endian.h>

class Codec {
public:
	typedef std::function<void (const nutty::TcpConnectionPtr&, const std::string& message)> MessageCallback;

	explicit Codec(const MessageCallback& cb)
		: messageCallback_(cb) {
	}

	void onRead(const nutty::TcpConnectionPtr& conn, nutty::ReceiveBuffer& receiveBuffer) {
		while (receiveBuffer.size() >= kHeaderLen) {
			int32_t be32;
			receiveBuffer.peek(&be32, sizeof(int32_t));
			const int32_t len = be32toh(be32);

			if (len > 65536 || len < 0) {
				conn->shutdown();
				break;
			} else if (receiveBuffer.size() >= len + kHeaderLen) {
				receiveBuffer.retrieve(kHeaderLen);
				std::string message;
				message.reserve(len);
				receiveBuffer.peek(message, len);
				messageCallback_(conn, message);
				receiveBuffer.retrieve(len + kHeaderLen);
			} else {
				break;
			}
		}
	}

	void send(nutty::TcpConnectionPtr& conn, const std::string& message) {
		int32_t len = static_cast<int32_t>(message.size());
		int32_t be32 = htobe32(len);

		std::string buffer;
		buffer.reserve(len + kHeaderLen);
		buffer.append(reinterpret_cast<const char*>(&be32), sizeof(int32_t));
		buffer.append(message);

		conn->send(buffer);
	}

private:
	const static size_t kHeaderLen = sizeof(int32_t);

	MessageCallback messageCallback_;
}; // end class Codec

#endif // NUTTY_EXAMPLES_CHAT_CODEC_H