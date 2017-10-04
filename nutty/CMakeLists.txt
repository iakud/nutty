add_subdirectory(base)
# add_subdirectory(log)
add_subdirectory(net)
add_subdirectory(util)

add_library(nutty)
target_link_libraries(nutty nutty_base nutty_net nutty_util)

install(TARGETS nutty DESTINATION lib)