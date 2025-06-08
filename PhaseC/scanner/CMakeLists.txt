find_package(FLEX QUIET)
if (FLEX_FOUND)
    message(STATUS "Found FLEX: ${FLEX_EXECUTABLE} (found version ${FLEX_VERSION})")
else ()
    message(FATAL_ERROR "Could not find FLEX. Please install `flex`.")
endif ()

set(ALPHA_SCANNER_OUTPUT_FILEPATH ${AUTOGEN_SRC_DIR_PATH}/${FLEX_OUTPUT_FILENAME})
set(ALPHA_SCANNER_HEADER_FILEPATH ${AUTOGEN_INCLUDE_DIR_PATH}/${FLEX_HEADER_FILENAME})
FLEX_TARGET(ALPHA_SCANNER
    src/${FLEX_SPEC_FILENAME}
    ${ALPHA_SCANNER_OUTPUT_FILEPATH}
    COMPILE_FLAGS "--header-file=${ALPHA_SCANNER_HEADER_FILEPATH}"
)
# Opposed to bison, flex doesn't have a way to track its produced header file, so we track it manually.
set_source_files_properties(${ALPHA_SCANNER_HEADER_FILEPATH} PROPERTIES GENERATED TRUE)
# Ensure `make clean` removes it
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${ALPHA_SCANNER_HEADER_FILEPATH}")

# We create a phony target so we force FLEX files to be generated before compilation.
add_custom_target(generate_scanner ALL DEPENDS "${FLEX_ALPHA_SCANNER_OUTPUTS}")

set_source_files_properties(
    ${ALPHA_SCANNER_OUTPUT_FILEPATH}
    PROPERTIES COMPILE_FLAGS "-Wno-implicit-fallthrough"
)

add_library(alpha_scanner_api INTERFACE)
target_include_directories(alpha_scanner_api INTERFACE
    "${AUTOGEN_INCLUDE_DIR_PATH}"
    "include"
)

add_library(alpha_scanner_impl STATIC
    ${ALPHA_SCANNER_OUTPUT_FILEPATH}
    src/scanner_token.cpp
)
target_compile_options(alpha_scanner_impl PRIVATE
    -Wno-unused-function
)
target_link_libraries(alpha_scanner_impl PRIVATE
    alpha_scanner_api
    alpha_parser_api
    alpha_core_api
)

add_dependencies(alpha_scanner_impl generate_parser_scanner)
