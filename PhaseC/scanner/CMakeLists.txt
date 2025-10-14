# ---------------------------------------------------------------------------
# Check Flex dependency for generating scanner
# ---------------------------------------------------------------------------
find_package(FLEX QUIET)
if (FLEX_FOUND)
    log_info("Found FLEX: ${FLEX_EXECUTABLE} (found version ${FLEX_VERSION})")
else ()
    message(FATAL_ERROR "Could not find FLEX. Please install `flex`.")
endif ()

# ---------------------------------------------------------------------------
# Setup paths for generated scanner (headers + source)
# ---------------------------------------------------------------------------
get_filename_component(CURRENT_COMPONENT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(GENERATED_INCLUDE_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/include")
set(GENERATED_INCLUDE_COMPONENT_DIR "${GENERATED_INCLUDE_DIR}/${CURRENT_COMPONENT_NAME}")
set(GENERATED_SRC_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/src")

make_dir_verbose("${GENERATED_INCLUDE_DIR}")
make_dir_verbose("${GENERATED_INCLUDE_COMPONENT_DIR}")
make_dir_verbose("${GENERATED_SRC_DIR}")

# ---------------------------------------------------------------------------
# Configure lexical scanner (Flex) and output paths
# ---------------------------------------------------------------------------
set(ALPHA_SCANNER_OUTPUT_FILEPATH "${GENERATED_SRC_DIR}/${FLEX_OUTPUT_FILENAME}")
set(ALPHA_SCANNER_HEADER_FILEPATH "${GENERATED_INCLUDE_COMPONENT_DIR}/${FLEX_HEADER_FILENAME}")

flex_target(ALPHA_SCANNER
    "${CMAKE_CURRENT_SOURCE_DIR}/src/${FLEX_SPEC_FILENAME}"
    "${ALPHA_SCANNER_OUTPUT_FILEPATH}"
    COMPILE_FLAGS "--header-file=${ALPHA_SCANNER_HEADER_FILEPATH}"
)
# Opposed to bison, flex doesn't have a way to track its produced header file, so we track it manually.
set_source_files_properties(${ALPHA_SCANNER_HEADER_FILEPATH} PROPERTIES GENERATED TRUE)
# Ensure `make clean` removes it
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${ALPHA_SCANNER_HEADER_FILEPATH}")

# We create a phony target so we force FLEX files to be generated before compilation.
add_custom_target(generate_scanner DEPENDS "${FLEX_ALPHA_SCANNER_OUTPUTS}")

set_source_files_properties(
    ${ALPHA_SCANNER_OUTPUT_FILEPATH}
    PROPERTIES COMPILE_FLAGS "-Wno-implicit-fallthrough"
)

add_library(alpha_scanner_api INTERFACE)
target_include_directories(alpha_scanner_api INTERFACE
    "${GENERATED_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_library(alpha_scanner_impl STATIC
    ${ALPHA_SCANNER_OUTPUT_FILEPATH}
    src/scanner_context.cpp
    src/scanner_token.cpp
)
target_compile_options(alpha_scanner_impl PRIVATE
    -Wno-unused-function
)

target_include_directories(alpha_scanner_impl PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(alpha_scanner_impl PRIVATE
    alpha_scanner_api
    alpha_parser_api
    alpha_core_api
    alpha_diagnostics_api
)

add_dependencies(alpha_scanner_impl generate_parser_scanner)
