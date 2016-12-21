add_executable(Singleton_test Singleton_test.cpp)
target_link_libraries(Singleton_test nutty_util)

add_executable(Semaphore_test Semaphore_test.cpp)
target_link_libraries(Semaphore_test nutty_util)

add_executable(CountDownLatch_test CountDownLatch_test.cpp)
target_link_libraries(CountDownLatch_test nutty_util)

add_executable(BlockingQueue_test BlockingQueue_test.cpp)
target_link_libraries(BlockingQueue_test nutty_util)

add_executable(BoundedBlockingQueue_test BoundedBlockingQueue_test.cpp)
target_link_libraries(BoundedBlockingQueue_test nutty_util)