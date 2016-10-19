cmake_minimum_required(VERSION 2.6)

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CATTABASE_SRCS)
set(CATTABASE_SRCS
	Thread.cpp
	)

add_library(cattabase ${CATTABASE_SRCS})
set_target_properties(cattabase PROPERTIES COMPILE_FLAGS "-std=c++14")

install(TARGETS cattabase DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/catta/base)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
	add_subdirectory(tests)
endif()
