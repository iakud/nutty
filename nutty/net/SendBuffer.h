#ifndef NUTTY_NET_SENDBUFFER_H
#define NUTTY_NET_SENDBUFFER_H

#include <cstdint>
#include <deque>

struct iovec;

namespace nutty {

class ReceiveBuffer;
class Buffer;

class SendBuffer {
public:
	SendBuffer();
	~SendBuffer();

	void append(const void* buf, uint32_t count);
	void append(Buffer&& buf);
	void append(const ReceiveBuffer& receiveBuffer);
	void append(const ReceiveBuffer& receiveBuffer, uint32_t offset);

	uint32_t size() const { return size_; }

private:
	static const uint32_t kSendSize = 16 * 1024;

	SendBuffer(const SendBuffer&) = delete;
	SendBuffer& operator=(const SendBuffer&) = delete;

	inline int buffersSize() const { return static_cast<int>(buffers_.size()); }
	void prepareSend(struct iovec* iov, int& iovcnt) const;
	void hasSent(uint32_t count);

	std::deque<Buffer*> buffers_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class SendBuffer

} // end namespace nutty

#endif // NUTTY_NET_SENDBUFFER_H