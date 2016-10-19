#include <catta/base/Thread.h>

#include <unistd.h>
#include <sys/syscall.h>

namespace catta {

thread_local int t_cachedTid = 0;

pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

} // end namespace catta

using namespace catta;

void CurrentThread::cacheTid() {
	if (t_cachedTid == 0) {
		t_cachedTid = gettid();
	}
}

bool CurrentThread::isMainThread() {
	return tid() == ::getpid();
}