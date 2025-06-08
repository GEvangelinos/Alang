# === arguinator_api: Header-only interface ===
add_library(arguinator_api INTERFACE)

target_include_directories(arguinator_api INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# === arguinator_impl: Implementation ===
add_library(arguinator_impl STATIC
    src/arguinator.cpp
)

target_link_libraries(arguinator_impl PRIVATE
    arguinator_api
    utils
)
