#ifndef NUTTY_NET_BUFFER_H
#define NUTTY_NET_BUFFER_H

#include <cstdint>
#include <deque>

struct iovec;

namespace nutty {

class Buffer {
public:
	Buffer(uint32_t capacity);
	Buffer(const void* buf, uint32_t count);
	Buffer(Buffer&& buffer);
	~Buffer();

	inline char* data() const { return buf_; }
	inline char* dataRead() const { return buf_ + rpos_; }
	inline char* dataWrite() const { return buf_ + wpos_; }

	inline uint32_t capacity() const { return cap_; }
	inline uint32_t readableSize() const { return wpos_ - rpos_; }
	inline uint32_t writableSize() const { return cap_ - wpos_; }

	inline void hasRead(uint32_t count) { rpos_ += count; }
	inline void hasWritten(uint32_t count) { wpos_ += count; }
	inline void reset() { rpos_ = wpos_ = 0; }
	inline bool empty() const { return rpos_ == wpos_; }
	inline bool full() const { return wpos_ == cap_; }

private:
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	char* buf_;
	uint32_t cap_;
	uint32_t rpos_;
	uint32_t wpos_;
}; // end class Buffer

class SendBuffer {
public:
	SendBuffer();
	~SendBuffer();

	void append(const void* buf, uint32_t count);
	void append(Buffer&& buf);

	uint32_t size() const { return size_; }

private:
	static const uint32_t kSendSize = 8 * 1024;

	SendBuffer(const SendBuffer&) = delete;
	SendBuffer& operator=(const SendBuffer&) = delete;

	inline int buffersSize() const { return static_cast<int>(buffers_.size()); }
	void prepareSend(struct iovec* iov, int& iovcnt) const;
	void hasSent(uint32_t count);

	std::deque<Buffer*> buffers_;
	uint32_t size_;

	friend class TcpConnection;
}; // end class SendBuffer

class ReceiveBuffer {
public:
	ReceiveBuffer();
	~ReceiveBuffer();

	void read(void* buf, uint32_t count);
	void peek(void* buf, uint32_t count) const;
	void peek(void* buf, uint32_t offset, uint32_t count) const;
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

#endif // NUTTY_NET_BUFFER_H