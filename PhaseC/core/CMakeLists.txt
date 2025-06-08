add_library(alpha_core_api INTERFACE)
target_include_directories(alpha_core_api INTERFACE "include")
target_link_libraries(alpha_core_api INTERFACE utils_api)


add_library(alpha_core_impl STATIC
    src/diagnostics.cpp
    src/source_location.cpp
)
target_link_libraries(alpha_core_impl PRIVATE alpha_core_api)
