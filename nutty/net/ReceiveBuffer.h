#ifndef NUTTY_NET_RECEIVEBUFFER_H
#define NUTTY_NET_RECEIVEBUFFER_H

#include <deque>
#include <string>
#include <cstddef>

struct iovec;

namespace nutty {

class Buffer;

class ReceiveBuffer {
public:
	ReceiveBuffer();
	~ReceiveBuffer();

	void read(void* buf, size_t count);
	void read(std::string& buf, size_t count);
	void peek(void* buf, size_t count) const;
	void peek(std::string& buf, size_t count) const;
	void peek(void* buf, size_t offset, size_t count) const;
	void peek(std::string& buf, size_t offset, size_t count) const;
	void retrieve(size_t count);
	void retrieveAll();

	size_t size() const { return size_; }

private:
	static const size_t kReceiveSize = 8 * 1024;

	ReceiveBuffer(const ReceiveBuffer&) = delete;
	ReceiveBuffer& operator=(const ReceiveBuffer&) = delete;

	void prepareReceive(struct iovec* iov, int& iovcnt);
	void hasReceived(size_t count);
	inline size_t buffersSize() const { return buffers_.size(); }
	void prepareSend(struct iovec* iov, int& iovcnt) const;

	std::deque<Buffer*> buffers_;
	std::deque<Buffer*> extendBuffers_;
	size_t size_;

	friend class TcpConnection;
}; // end class ReceiveBuffer

} // end namespace nutty

#endif // NUTTY_NET_RECEIVEBUFFER_H