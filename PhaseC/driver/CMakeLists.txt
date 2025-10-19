get_filename_component(CURRENT_COMPONENT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(GENERATED_INCLUDE_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/include")

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
    src/exception.cpp
    src/translation_unit_buffer.cpp
)
target_include_directories(alpha_driver_impl PRIVATE
    "${GENERATED_INCLUDE_DIR}"
)
target_link_libraries(alpha_driver_impl PRIVATE
    alpha_core_api
    alpha_driver_api
    alpha_ir_optimizer_api
    alpha_settings_api
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
    alpha_ir_optimizer_impl
)

# === Custom generation step (scanner + parser) ===
add_custom_target(generate_parser_scanner ALL DEPENDS
    generate_scanner
    generate_parser
)

add_dependencies(${COMPILER_EXE_NAME}
    generate_parser_scanner
)

target_compile_options(${COMPILER_EXE_NAME} PRIVATE -flto)
target_link_options(${COMPILER_EXE_NAME} PRIVATE -flto)