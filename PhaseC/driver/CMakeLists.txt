# === alpha_driver_api: Interface (public headers only) ===
add_library(alpha_driver_api INTERFACE)
target_include_directories(alpha_driver_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(alpha_driver_api INTERFACE
    alpha_core_api
    alpha_parser_api
    alpha_scanner_api
)

# === alpha_driver_impl: Implementation (compiled source) ===
add_library(alpha_driver_impl STATIC
    src/alpha_driver.cpp
)
target_include_directories(alpha_driver_impl PRIVATE
    "${AUTOGEN_INCLUDE_DIR_PATH}"
)
target_link_libraries(alpha_driver_impl PRIVATE
    alpha_driver_api
    alpha_core_api
)

# === Executable ===
add_executable("${ALPHA_DRIVER_EXECUTABLE}"
    src/app_main.cpp
)
target_link_libraries("${ALPHA_DRIVER_EXECUTABLE}" PRIVATE
    arguinator_api
    alpha_driver_api

    arguinator_impl
    alpha_driver_impl
    alpha_core_impl
    alpha_parser_impl
    alpha_scanner_impl
)

# === Custom generation step (scanner + parser) ===
add_custom_target(generate_parser_scanner ALL DEPENDS
    generate_parser
    generate_scanner
)

add_dependencies(${ALPHA_DRIVER_EXECUTABLE}
    generate_parser
    generate_scanner
)
