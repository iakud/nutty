#ifndef NUTTY_NET_SENDBUFFER_H
#define NUTTY_NET_SENDBUFFER_H

#include <deque>
#include <cstddef>

struct iovec;

namespace nutty {

class ReceiveBuffer;
class Buffer;

class SendBuffer {
public:
	SendBuffer();
	~SendBuffer();

	void append(const void* buf, size_t count);
	void append(Buffer&& buf);
	void append(const ReceiveBuffer& receiveBuffer);
	void append(const ReceiveBuffer& receiveBuffer, size_t offset);

	size_t size() const { return size_; }

private:
	static const size_t kSendSize = 16 * 1024;

	SendBuffer(const SendBuffer&) = delete;
	SendBuffer& operator=(const SendBuffer&) = delete;

	inline size_t buffersSize() const { return buffers_.size(); }
	void prepareSend(struct iovec* iov, int& iovcnt) const;
	void hasSent(size_t count);

	std::deque<Buffer*> buffers_;
	size_t size_;

	friend class TcpConnection;
}; // end class SendBuffer

} // end namespace nutty

#endif // NUTTY_NET_SENDBUFFER_H