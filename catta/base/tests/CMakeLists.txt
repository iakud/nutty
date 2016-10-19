add_executable(Thread_test Thread_test.cpp)
set_target_properties(Thread_test PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(Thread_test cattabase)