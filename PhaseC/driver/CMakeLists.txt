# Create a custom target 'generate_parser_scanner' that is built by default (ALL).
# It depends on both 'generate_parser' and 'generate_scanner', ensuring that the
# parser and scanner generation steps complete before any targets that rely on them are built.
add_custom_target(generate_parser_scanner ALL DEPENDS
    generate_parser
    generate_scanner
)

add_library(alpha_driver STATIC src/alpha_driver.cpp)
target_include_directories(alpha_driver PUBLIC "include")
target_link_libraries(alpha_driver PRIVATE
    alpha_core
    alpha_parser
    alpha_scanner
)

add_executable(${ALPHA_DRIVER_EXECUTABLE} src/app_main.cpp)
target_include_directories(${ALPHA_DRIVER_EXECUTABLE} PRIVATE
    "${AUTOGEN_INCLUDE_DIR_PATH}"
    "include"
)
target_link_libraries(${ALPHA_DRIVER_EXECUTABLE} PRIVATE
    arguinator_api
    arguinator_impl
    alpha_driver
)
add_dependencies(${ALPHA_DRIVER_EXECUTABLE}
    generate_scanner
    generate_parser
)
