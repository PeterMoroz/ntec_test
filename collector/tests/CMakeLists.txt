include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)


add_executable(test_queue test_queue.cpp)
target_link_libraries(test_queue gtest gtest_main collector)

add_executable(test_rule test_rule.cpp)
target_link_libraries(test_rule gtest gtest_main collector)
