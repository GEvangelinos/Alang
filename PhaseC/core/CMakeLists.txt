add_library(alpha_core STATIC
            src/alpha_diagnostics.cpp
            src/alpha_location.cpp
)

target_include_directories(alpha_core PRIVATE
        "include"
)
