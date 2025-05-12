add_subdirectory(arguinator)
add_subdirectory(core)
add_subdirectory(parser)
add_subdirectory(scanner)

# Create a custom target 'generate_parser_scanner' that is built by default (ALL).
# It depends on both 'generate_parser' and 'generate_scanner', ensuring that the
# parser and scanner generation steps complete before any targets that rely on them are built.

add_custom_target(generate_parser_scanner ALL DEPENDS
        generate_parser
        generate_scanner
)
add_executable(${ALPHA_DRIVER_EXECUTABLE}
        alpha_driver.cpp
        app_main.cpp
)

# target_link_options(${ALPHA_DRIVER_EXECUTABLE} PRIVATE -fsanitize=address)

add_dependencies(${ALPHA_DRIVER_EXECUTABLE}
        generate_scanner
        generate_parser)

target_link_libraries(${ALPHA_DRIVER_EXECUTABLE}
        PUBLIC
        arguinator
        alpha_core
        alpha_parser
        alpha_scanner)


target_include_directories(${ALPHA_DRIVER_EXECUTABLE} PRIVATE
        "${AUTOGEN_INCLUDE_DIR_PATH}"
        "${CMAKE_SOURCE_DIR}/include"
)
