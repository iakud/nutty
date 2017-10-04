#include "AsyncLogger.h"
#include "AsyncLogging.h"
#include "LogFile.h"
#include "Logging.h" // strerror_tl

#include <base/ProcessInfo.h>

#include <chrono>
#include <memory.h>
#include <assert.h>


namespace iak {
namespace log {

class AsyncLogger::Buffer {

private:
	static const int kBufferSize = 4096 * 1024;

public:
	Buffer()
		: cur_(data_)
		, end_(data_ + sizeof data_) {
	}

	~Buffer() {
	}
	// noncopyable
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	void append(const char* buf, size_t len) {
		if (static_cast<size_t>(avail()) > len) {
			::memcpy(cur_, buf, len);
			cur_ += len;
		}
	}

	const char* data() const {
		return data_;
	}

	int length() const {
		return static_cast<int>(cur_ - data_);
	}

	int avail() const { return static_cast<int>(end_ - cur_); }

	void reset() { cur_ = data_; }

private:
	char data_[kBufferSize];
	char* cur_;
	char* end_;
}; // end class AsyncLogger::Buffer

} // end namespace log
} // end namespace iak

using namespace iak::base;
using namespace iak::log;

AsyncLoggerPtr AsyncLogger::make(AsyncLogging* asyncLogging,
		const std::string& basename,
		size_t rollSize,
		bool hostNameInLogFileName,
		bool pidInLogFileName) {
	return std::make_shared<AsyncLogger>(asyncLogging, basename,
			rollSize, hostNameInLogFileName, pidInLogFileName);
}

AsyncLogger::AsyncLogger(AsyncLogging* asyncLogging,
		const std::string& basename,
		size_t rollSize,
		bool hostNameInLogFileName,
		bool pidInLogFileName)
	: asyncLogging_(asyncLogging)
	, basename_(basename)
	, rollSize_(rollSize)
	, hostNameInLogFileName_(hostNameInLogFileName)
	, pidInLogFileName_(pidInLogFileName)
	, count_(0)
	, startOfPeriod_(0)
	, lastRoll_(0)
	, lastFlush_(0)
	, notEmpty_(false)
	, currentBuffer_(std::make_shared<Buffer>())
	, nextBuffer_(std::make_shared<Buffer>())
	, buffers_()
	, buffersToWrite_()
	, buffer1_(std::make_shared<Buffer>())
	, buffer2_(std::make_shared<Buffer>()) {
	assert(basename.find('/') == std::string::npos);
	rollFile();
}

AsyncLogger::~AsyncLogger() {
}

void AsyncLogger::append(const char* logline, int len) {
	asyncLogging_->append(shared_from_this(), logline, len);
}

void AsyncLogger::flush() {
}

void AsyncLogger::appendToBuffer_locked(const char* logline, int len) {
	if (currentBuffer_->avail() > len) {
		currentBuffer_->append(logline, len);
	} else {
		buffers_.push_back(currentBuffer_);
		if (nextBuffer_) {
			currentBuffer_ = nextBuffer_;
			nextBuffer_.reset();
		} else {
			currentBuffer_ = std::make_unique<Buffer>(); // Rarely happens
		}
		currentBuffer_->append(logline, len);
	}
	if (!notEmpty_) {
		notEmpty_ = true;
	}
}

void AsyncLogger::swapBuffersToWrite_locked() {
	if (currentBuffer_->length() > 0) {
		buffers_.push_back(currentBuffer_);
		currentBuffer_ = buffer1_;
		buffer1_.reset();
	}
	if (!nextBuffer_) {
		nextBuffer_.swap(buffer2_);
	}
	buffersToWrite_.swap(buffers_);
	if (notEmpty_) {
		notEmpty_ = false;
	}
}

void AsyncLogger::appendBuffers_unlocked() {
	if (buffersToWrite_.size() > 25) {
		/*
		char buf[256];
		::snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
			Timestamp::now().toFormattedString().c_str(),
			buffersToWrite.size() - 2);
		::fputs(buf, stderr);
		append_unlocked(buf, static_cast<int>(strlen(buf)));
		*/
		//buffersToWrite_.erase(buffersToWrite_.begin() + 2, buffersToWrite_.end());
	}

	for (const BufferPtr& buffer : buffersToWrite_) {
		append_unlocked(buffer->data(), buffer->length());
	}
	if (!buffer1_) {
		buffer1_ = buffersToWrite_.back();
		buffersToWrite_.pop_back();
		buffer1_->reset(); // buffer's func reset
	}
	if (!buffer2_) {
		buffer2_ = buffersToWrite_.back();
		buffersToWrite_.pop_back();
		buffer2_->reset(); // buffer's func reset
	}
	buffersToWrite_.clear();
	// flush
	logFile_->flush();
}

void AsyncLogger::append_unlocked(const char* logline, int len) {
	logFile_->append(logline, len);
	if (logFile_->writtenBytes() > rollSize_) {
		count_ = 0;
		rollFile();
	} else {
		if (count_ > kCheckTimeRoll_) {
			count_ = 0;
			time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
			if (thisPeriod_ != startOfPeriod_) {
				rollFile();
			}
		} else {
			++count_;
		}
	}
}

void AsyncLogger::rollFile() {
	time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if (now > lastRoll_) {
		lastRoll_ = now;
		lastFlush_ = now;
		startOfPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;

		std::string filename;
		const std::string& destination = Logging::getLogDestination();
		filename.reserve(destination.size() + basename_.size() + 64);
		filename = destination + basename_;

		struct tm tm_now;
		::localtime_r(&now, &tm_now);
		char timebuf[32];
		::strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm_now);
		filename += timebuf;
		if (hostNameInLogFileName_) {
			filename += ".";
			filename += ProcessInfo::hostName();
		}
		if (pidInLogFileName_) {
			char pidbuf[32];
			::snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
			filename += pidbuf;
		}
		filename += ".log";
		logFile_ = std::make_unique<LogFile>(filename);
	}
}