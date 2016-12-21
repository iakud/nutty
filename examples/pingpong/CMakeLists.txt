add_executable(pingpong_client client.cpp)
target_link_libraries(pingpong_client nutty)

add_executable(pingpong_server server.cpp)
target_link_libraries(pingpong_server nutty)