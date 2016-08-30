cmake_minimum_required(VERSION 2.6)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CATTANET_SRCS)
add_library(cattanet ${CATTANET_SRCS})
set_target_properties(cattanet PROPERTIES COMPILE_FLAGS "-std=c++14")

install(TARGETS cattanet DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/catta/net)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
	add_subdirectory(tests)
endif()
