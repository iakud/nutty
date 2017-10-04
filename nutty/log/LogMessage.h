#ifndef NUTTY_LOG_LOGMESSAGE_H
#define NUTTY_LOG_LOGMESSAGE_H

namespace nutty {

enum class LogLevel {
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL,
	NUM_LOG_LEVELS
};

template<class LOGGER>
class LogMessage {
public:
	LogMessage(LOGGER& logger, LogLevel level);
	LogMessage(const char* filename, int line, LogLevel level);
	LogMessage(const char* filename, int line, LogLevel level, const char* func);
	~LogMessage();

	void stream() {}
private:
	LOGGER& logger_;
}; // end class LogMessage

#define NUTTY_TRACE ::nutty::LogMessage(__FILE__, __LINE__, ::nutty::LogLevel::TRACE, __func__).stream()
#define NUTTY_DEBUG ::nutty::LogMessage(__FILE__, __LINE__, ::nutty::LogLevel::DEBUG, __func__).stream()
#define NUTTY_INFO ::nutty::LogMessage(__FILE__, __LINE__, ::nutty::LogLevel::INFO).stream()
#define NUTTY_WARN ::nutty::LogMessage(__FILE__, __LINE__, ::nutty::LogLevel::WARN).stream()
#define NUTTY_ERROR ::nutty::LogMessage(__FILE__, __LINE__, ::nutty::LogLevel::ERROR).stream()
#define NUTTY_FATAL ::nutty::LogMessage(__FILE__, __LINE__, ::nutty::LogLevel::FATAL).stream()

#define LOG_DEBUG(LOGGER) ::nutty::LogMessage((LOGGER), ::nutty::LogLevel::DEBUG).stream()
#define LOG_INFO(LOGGER) ::nutty::LogMessage((LOGGER), ::nutty::LogLevel::INFO).stream()
#define LOG_WARN(LOGGER) ::nutty::LogMessage((LOGGER), ::nutty::LogLevel::WARN).stream()
#define LOG_ERROR(LOGGER) ::nutty::LogMessage((LOGGER), ::nutty::LogLevel::ERROR).stream()

} // end namespace nutty

#endif // NUTTY_LOG_LOGMESSAGE_H