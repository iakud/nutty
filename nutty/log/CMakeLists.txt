cmake_minimum_required(VERSION 2.8)

set(NUTTY_LOG_SRCS
	LogFormat.cpp
#	LogStream.cpp
	LogMessage.cpp
	)

add_library(nutty_log ${NUTTY_LOG_SRCS})
target_link_libraries(nutty_log nutty_base)

# install(TARGETS nutty_log DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/nutty/log)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
	add_subdirectory(tests)
endif()