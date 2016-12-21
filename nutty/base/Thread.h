#ifndef NUTTY_BASE_CURRENTTHREAD_h
#define NUTTY_BASE_CURRENTTHREAD_h

#include <unistd.h>

namespace nutty {

extern thread_local pid_t t_cachedTid;

class CurrentThread {
public:
	inline static pid_t tid() {
		if (__builtin_expect(t_cachedTid == 0, 0)) {
			t_cachedTid = getTid();
		}
		return t_cachedTid;
	}

	bool isMainThread();

private:
	static pid_t getTid();
};

} // end namespace nutty

#endif // NUTTY_BASE_CURRENTTHREAD_h