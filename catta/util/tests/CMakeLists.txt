add_executable(singleton_test singleton_test.cpp)
set_target_properties(singleton_test PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(singleton_test cattautil)
