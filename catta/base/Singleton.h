#ifndef CATTA_UTIL_SINGLETON_H
#define CATTA_UTIL_SINGLETON_H

#include <catta/base/noncopyable.h>

#include <mutex>
#include <cstdlib> // std::atexit

namespace catta {

template<typename T>
class Singleton : noncopyable {
public:
	static T& instance() {
		std::call_once(s_once_, Singleton::init);
		return *s_value_;
	}

private:
	Singleton();
	~Singleton();

	struct has_no_destroy {
		template <typename U>
		static auto check(int) -> decltype(std::declval<U>().no_destroy(), std::true_type());

		template <typename U>
		static std::false_type check(...);

		enum { value = std::is_same<decltype(check<T>(0)), std::true_type>::value };
	}; // end struct has_no_destroy

	static void init() {
		s_value_ = new T();

		if (!has_no_destroy::value) {
			std::atexit(destroy);
		}
	}

	static void destroy() {
		typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
		T_must_be_complete_type dummy; (void) dummy;
		delete s_value_;
		s_value_ = nullptr;
	}

	static std::once_flag s_once_;
	static T* s_value_;
}; // end class Singleton

template<typename T>
std::once_flag Singleton<T>::s_once_;

template<typename T>
T* Singleton<T>::s_value_ = nullptr;

} // end namespace catta

#endif // CATTA_UTIL_SINGLETON_H
