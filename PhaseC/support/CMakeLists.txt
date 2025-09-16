add_library(support_api INTERFACE)
target_include_directories(support_api INTERFACE include)
target_link_libraries(support_api INTERFACE fmt_api)

add_library(support_impl STATIC
    src/string_tools.cpp
    src/size_format_tools.cpp
)
target_link_libraries(support_impl PRIVATE support_api)

