add_executable(chat_client client.cpp)
target_link_libraries(chat_client nutty)

add_executable(chat_server server.cpp)
target_link_libraries(chat_server nutty)