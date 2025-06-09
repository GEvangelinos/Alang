add_library(alpha_diagnostics_api INTERFACE)
target_include_directories(alpha_diagnostics_api INTERFACE include)
target_link_libraries(alpha_diagnostics_api INTERFACE alpha_core_api)

add_library(alpha_diagnostics_impl STATIC
    src/diagnostics.cpp
    src/diagnostic_engine.cpp
)
target_link_libraries(alpha_diagnostics_impl PRIVATE
    alpha_diagnostics_api
    alpha_core_api
)