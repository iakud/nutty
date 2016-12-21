#include <nutty/base/Thread.h>

#include <sys/syscall.h>

namespace nutty {

thread_local pid_t t_cachedTid = 0;

} // end namespace nutty

using namespace nutty;

pid_t CurrentThread::getTid() {
	return static_cast<pid_t>(::syscall(SYS_gettid));
}

bool CurrentThread::isMainThread() {
	return tid() == ::getpid();
}