add_library(alpha_core STATIC
        errorTracker.cpp
)

target_include_directories(alpha_core PUBLIC
        ${CMAKE_SOURCE_DIR}/include/core
)
