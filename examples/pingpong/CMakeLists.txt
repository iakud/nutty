add_executable(pingpong_client client.cpp)
set_target_properties(pingpong_client PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(pingpong_client cattanet)

add_executable(pingpong_server server.cpp)
set_target_properties(pingpong_server PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(pingpong_server cattanet)