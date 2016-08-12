cmake_minimum_required(VERSION 2.6)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CATTAUTIL_SRCS)
add_library(cattautil ${CATTAUTIL_SRCS})
set_target_properties(cattautil PROPERTIES COMPILE_FLAGS "-std=c++14")

install(TARGETS cattautil DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/catta/util)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
	add_subdirectory(tests)
endif()