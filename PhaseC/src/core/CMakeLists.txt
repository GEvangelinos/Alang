add_library(alpha_core STATIC
        alpha_diagnostics.cpp
        alpha_location.cpp
)

target_include_directories(alpha_core PRIVATE
        "${CMAKE_SOURCE_DIR}/include")

