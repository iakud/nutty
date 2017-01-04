cmake_minimum_required(VERSION 2.8)

set(NUTTY_BASE_SRCS
	Thread.cpp
	EPollPoller.cpp
	Watcher.cpp
	Timer.cpp
	TimerQueue.cpp
	EventLoop.cpp
	EventLoopThread.cpp
	EventLoopThreadPool.cpp
	)

add_library(nutty_base ${NUTTY_BASE_SRCS})

# install(TARGETS nutty_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/nutty/base)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
	add_subdirectory(tests)
endif()