# === alpha_driver_api: Interface (public headers only) ===
add_library(alpha_driver_api INTERFACE)
target_include_directories(alpha_driver_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(alpha_driver_api INTERFACE
    alpha_core_api
    alpha_parser_api
    alpha_scanner_api
    alpha_diagnostics_api
)


# === alpha_driver_impl: Implementation (compiled source) ===
add_library(alpha_driver_impl STATIC
    src/alpha_driver.cpp
    src/translation_unit.cpp
    src/translation_unit_buffer_loader.cpp
    src/exception.cpp
)
target_link_libraries(alpha_driver_impl PRIVATE
    alpha_core_api
    alpha_driver_api
    alpha_ir_postprocess_api
    alpha_settings_api
    alpha_bytecode_api
    arguinator_api
)

# === Executable ===
add_executable("${COMPILER_EXE_NAME}"
    src/main.cpp
)
target_link_libraries("${COMPILER_EXE_NAME}" PRIVATE
    arguinator_api
    alpha_driver_api
    alpha_settings_api

    arguinator_impl
    support_impl
    alpha_driver_impl
    alpha_core_impl
    alpha_diagnostics_impl
    alpha_parser_impl
    alpha_scanner_impl
    alpha_settings_impl
    alpha_ir_postprocess_impl
    alpha_bytecode_impl
)

# === Custom generation step (scanner + parser + IR::Opcodes) ===
add_custom_target(generate_all ALL DEPENDS
    generate_scanner
    generate_parser
    generate_ir_opcodes
)
add_dependencies(${COMPILER_EXE_NAME} generate_all)
# Anchor generation to the core API targets so NO implementation can compile without them
add_dependencies(alpha_core_api generate_all)
add_dependencies(alpha_parser_api generate_all)
add_dependencies(alpha_scanner_api generate_all)

# Keep these as explicit sanity locks for the implementations
add_dependencies(alpha_bytecode_impl generate_all)
add_dependencies(alpha_parser_impl generate_all)
add_dependencies(alpha_scanner_impl generate_all)
add_dependencies(alpha_core_impl generate_all)

target_compile_options(${COMPILER_EXE_NAME} PRIVATE -flto)
target_link_options(${COMPILER_EXE_NAME} PRIVATE -flto)