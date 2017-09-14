#include <nutty/base/Poller.h>

#include <nutty/base/poller/PollPoller.h>
#include <nutty/base/poller/EPollPoller.h>

using namespace nutty;

Poller* Poller::createDefault() {
	// FIXME : use poll
	return new EPollPoller();
}