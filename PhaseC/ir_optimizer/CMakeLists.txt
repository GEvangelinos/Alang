add_library(alpha_ir_optimizer_api INTERFACE)
target_include_directories(alpha_ir_optimizer_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_library(alpha_ir_optimizer_impl STATIC
    src/ir_optimizer.cpp
)
target_link_libraries(alpha_ir_optimizer_impl PRIVATE
    alpha_ir_optimizer_api
    alpha_settings_api
    alpha_core_api
)

add_dependencies(alpha_ir_optimizer_impl generate_ir_opcodes)