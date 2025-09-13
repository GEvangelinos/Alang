add_library(utils_api INTERFACE)
target_include_directories(utils_api INTERFACE include)
target_link_libraries(utils_api INTERFACE fmt_api)

add_library(utils_impl STATIC
    src/string_utils.cpp
)
target_link_libraries(utils_impl PRIVATE utils_api)

