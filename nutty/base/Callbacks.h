#ifndef NUTTY_BASE_CALLBACKS_H
#define NUTTY_BASE_CALLBACKS_H

#include <functional>
#include <memory>

namespace nutty {

class Timer;
typedef std::shared_ptr<Timer> TimerPtr;

// callback typedef
typedef std::function<void()> TimerCallback;

} // end namespace nutty

#endif // NUTTY_BASE_CALLBACKS_H
