include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG "v3.11.1"
)

FetchContent_MakeAvailable(json)


FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG master
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
find_package(Threads)
target_link_libraries(asio INTERFACE Threads::Threads)



set(COLLECTOR_SRC
    events_observer.cpp
    events_processor.cpp
    sha256.cpp
    integrity_checker.cpp
    tcp_client.cpp
)

add_library(collector ${COLLECTOR_SRC})
target_link_libraries(collector PUBLIC nlohmann_json::nlohmann_json asio)

add_executable(collector_app "main.cpp")
target_link_libraries(collector_app collector)