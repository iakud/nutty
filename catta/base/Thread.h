#ifndef CATTA_BASE_CURRENTTHREAD_h
#define CATTA_BASE_CURRENTTHREAD_h

namespace catta {

extern thread_local int t_cachedTid;

class CurrentThread {
public:
	inline static int tid() {
		if (__builtin_expect(t_cachedTid == 0, 0)) {
			cacheTid();
		}
		return t_cachedTid;
	}

	bool isMainThread();

private:
	static void cacheTid();
};

} // end namespace catta

#endif // CATTA_BASE_CURRENTTHREAD_h