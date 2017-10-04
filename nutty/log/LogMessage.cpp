#include <nutty/log/LogMessage.h>

namespace nutty {

thread_local char t_time[32];
thread_local int64_t t_lastSecond;

const char* LogLevelName[static_cast<int>(LogLevel::NUM_LOG_LEVELS)] = {
	"TRACE ", "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL "
};

} // end namespace nutty

using namespace nutty;

template<class LOGGER>
LogMessage<LOGGER>::LogMessage(LOGGER& logger, const char* filename, int line, LogLevel level) {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
	time_t time = std::chrono::system_clock::to_time_t(now);
	if (seconds.count() != t_lastSecond) {
		t_lastSecond = seconds.count();
		struct tm tm_time;
		//::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
		::localtime_r(&time, &tm_time);
		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17); (void)len;
	}
	stream_ << t_time;
	// stream_ << Thread::tidString(); // FIXME:std::this_thread::get_id()
	stream_ << LogLevelName[static_cast<int>(level)];
}

template<class LOGGER>
LogMessage<LOGGER>::LogMessage(LOGGER& logger, const char* filename, int line, LogLevel level, const char* func) {
	
}

template<class LOGGER>
LogMessage<LOGGER>::LogMessage(const char* filename, int line, LogLevel level) {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
	time_t time = std::chrono::system_clock::to_time_t(now);
	if (seconds.count() != t_lastSecond) {
		t_lastSecond = seconds.count();
		struct tm tm_time;
		//::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
		::localtime_r(&time, &tm_time);
		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17); (void)len;
	}
	stream_ << t_time;
	// stream_ << Thread::tidString(); // FIXME:std::this_thread::get_id()
	stream_ << LogLevelName[static_cast<int>(level)];
}

template<class LOGGER>
LogMessage<LOGGER>::LogMessage(const char* filename, int line, LogLevel level, const char* func) {

}

template<class LOGGER>
LogMessage<LOGGER>::~LogMessage() {

}

// explicit instantiations
//template LogMessage::LogMessage(const char* fmt, char);