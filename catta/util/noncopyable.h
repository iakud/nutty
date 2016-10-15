#ifndef CATTA_NET_NONCOPYABLE_H
#define CATTA_NET_NONCOPYABLE_H

namespace catta {

class noncopyable {
protected:
	noncopyable() {}

private:
	noncopyable(const noncopyable&) = delete;
	noncopyable& operator=(const noncopyable&) = delete;
}; // end class noncopyable

} // end namespace catta

#endif // CATTA_NET_NONCOPYABLE_H