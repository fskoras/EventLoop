cmake_minimum_required(VERSION 3.10)

set(PROJECT_N EventLoop)
project(${PROJECT_N} VERSION 1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_library(${PROJECT_N} src/event_loop.c)
target_include_directories(${PROJECT_N} PUBLIC include)

# TESTS
add_executable(multi_producer_test example/multi_producer_test.c)
target_link_libraries(multi_producer_test PRIVATE ${PROJECT_N})
enable_testing()
add_test(multi-producer multi_producer_test)
