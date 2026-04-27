add_library(alpha_ir_postprocess_api INTERFACE)
target_include_directories(alpha_ir_postprocess_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_library(alpha_ir_postprocess_impl STATIC
    src/ir_optimizer.cpp
    src/ir_validator.cpp
)
target_link_libraries(alpha_ir_postprocess_impl PRIVATE
    alpha_ir_postprocess_api
    alpha_diagnostics_api
    alpha_settings_api
    alpha_core_api
)

add_dependencies(alpha_ir_postprocess_impl generate_ir_opcodes)