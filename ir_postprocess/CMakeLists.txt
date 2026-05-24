add_library(alpha_ir_postprocess_api INTERFACE)
target_include_directories(alpha_ir_postprocess_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_library(alpha_ir_postprocess_impl STATIC
    src/ir_optimizer.cpp
    src/ir_validator.cpp
    src/opts/ir_editor.cpp
    src/opts/opt_func_hoisting.cpp
    src/opts/opt_neighbor_jumps.cpp
    src/opts/opt_jump_threading.cpp
    src/opts/opt_dead_function_elimination.cpp
)
target_link_libraries(alpha_ir_postprocess_impl PRIVATE
    alpha_ir_postprocess_api
    alpha_diagnostics_api
    alpha_settings_api
    alpha_core_api
)

add_dependencies(alpha_ir_postprocess_impl generate_ir_opcodes)