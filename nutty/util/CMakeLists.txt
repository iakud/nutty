cmake_minimum_required(VERSION 2.8)

set(NUTTY_UTIL_SRCS
	CountDownLatch.cpp
	FileUtil.cpp
	)

add_library(nutty_util ${NUTTY_UTIL_SRCS})
# target_link_libraries(nutty_util nutty_base)

install(TARGETS nutty_util DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/nutty/util)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
	add_subdirectory(tests)
endif()