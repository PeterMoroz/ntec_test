include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG "v3.11.1"
)

FetchContent_MakeAvailable(json)

set(COLLECTOR_SRC
    events_observer.cpp
    events_processor.cpp
)

add_library(collector ${COLLECTOR_SRC})
target_link_libraries(collector PUBLIC nlohmann_json::nlohmann_json)

add_executable(collector_app "main.cpp")
target_link_libraries(collector_app collector)