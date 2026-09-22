cmake_minimum_required(VERSION 3.10)
project(ntec_test_events_collector)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(
    include
)

add_subdirectory(src)
add_subdirectory(tests)
