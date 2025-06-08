add_library(alpha_core STATIC
    src/diagnostics.cpp
    src/source_location.cpp
)
target_include_directories(alpha_core PUBLIC "include")
target_link_libraries(alpha_core PRIVATE utils)
