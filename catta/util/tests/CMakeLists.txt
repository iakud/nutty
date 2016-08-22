add_executable(Singleton_test Singleton_test.cpp)
set_target_properties(Singleton_test PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(Singleton_test cattautil)

add_executable(Semaphore_test Semaphore_test.cpp)
set_target_properties(Semaphore_test PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(Semaphore_test cattautil)

add_executable(CountDownLatch_test CountDownLatch_test.cpp)
set_target_properties(CountDownLatch_test PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(CountDownLatch_test cattautil)

add_executable(BlockingQueue_test BlockingQueue_test.cpp)
set_target_properties(BlockingQueue_test PROPERTIES LINK_FLAGS -pthread)
target_link_libraries(BlockingQueue_test cattautil)