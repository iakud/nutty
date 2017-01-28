#ifndef NUTTY_NET_RECEIVEBUFFER_H
#define NUTTY_NET_RECEIVEBUFFER_H

#include <cstdint>
#include <deque>

struct iovec;

namespace nutty {

class Buffer;

class ReceiveBuffer {
public:
	ReceiveBuffer();
	~ReceiveBuffer();

	void read(void* buf, uint32_t count);
	void peek(void* buf, uint32_t count) const;
	void peek(void* buf, uint32_t offset, uint32_t count) const;
	void retrieve(uint32_t count);
	void retrieveAll();

	uint32_t size() const { return size_; }

private:
	static const uint32_t kReceiveSize = 8 * 1024;

	ReceiveBuffer(const ReceiveBuffer&) = delete;
	ReceiveBuffer& operator=(const ReceiveBuffer&) = delete;

	void prepareReceive(struct iovec* iov, int& iovcnt);
	void hasReceived(uint32_t count);
	inline int buffersSize() const { return static_cast<int>(buffers_.size()); }
	void prepareSend(struct iovec* iov, int& iovcnt) const;

	std::deque<Buffer*> buffers_;
	std::deque<Buffer*> extendBuffers_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class ReceiveBuffer

} // end namespace nutty

#endif // NUTTY_NET_RECEIVEBUFFER_H