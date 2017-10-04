#ifndef NUTTY_LOG_ASYNCLOGGER_H
#define NUTTY_LOG_ASYNCLOGGER_H

#include <string>
#include <vector>
#include <memory>

namespace nutty {

class AsyncLogger;
typedef std::shared_ptr<AsyncLogger> AsyncLoggerPtr;

class LogFile;
class AsyncLogging;

class AsyncLogger : public std::enable_shared_from_this<AsyncLogger> {
public:
	friend class AsyncLogging;

	static AsyncLoggerPtr make(AsyncLogging* asyncLogging,
			const std::string& basename,
			size_t rollSize,
			bool hostNameInLogFileName = false,
			bool pidInLogFileName = false);

protected:
	static const int kCheckTimeRoll_ = 1024;
	static const int kRollPerSeconds_ = 60 * 60 * 24;

public:
	AsyncLogger(AsyncLogging* asyncLogging,
			const std::string& basename,
			size_t rollSize,
			bool hostNameInLogFileName = false,
			bool pidInLogFileName = false);
	~AsyncLogger();
	// noncopyable
	AsyncLogger(const AsyncLogger&) = delete;
	AsyncLogger& operator=(const AsyncLogger&) = delete;

	void append(const char* logline, int len);
	void flush();

private:
	bool isNotEmpty() { return notEmpty_; }
	void appendToBuffer_locked(const char* logline, int len);
	void swapBuffersToWrite_locked();
	void appendBuffers_unlocked();
	void append_unlocked(const char* logline, int len); // call in appendBuffers_unlocked
	void rollFile();

	AsyncLogging* asyncLogging_;
	const std::string basename_;
	const size_t rollSize_;

	bool hostNameInLogFileName_;
	bool pidInLogFileName_;

	int count_;
	time_t startOfPeriod_;
	time_t lastRoll_;
	time_t lastFlush_;

	std::unique_ptr<LogFile> logFile_;

	class Buffer;
	typedef std::shared_ptr<Buffer> BufferPtr;

	bool notEmpty_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	std::vector<BufferPtr> buffers_;
	std::vector<BufferPtr> buffersToWrite_;
	BufferPtr buffer1_; // backup
	BufferPtr buffer2_; // bakcup
}; // end class AsyncLogger

} // end namespace nutty

#include "LogMessage.h"

#endif  // NUTTY_LOG_ASYNCLOGGER_H
