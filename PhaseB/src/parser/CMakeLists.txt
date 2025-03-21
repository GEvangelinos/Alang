find_package(BISON QUIET)
if(BISON_FOUND)
        message(STATUS "Found BISON: ${BISON_EXECUTABLE} (found version ${BISON_VERSION})")
else()
        message(FATAL_ERROR "Could not find BISON. Please install `bison`.")
endif()

set(ALPHA_PARSER_OUTPUT_FILEPATH ${AUTOGEN_SRC_DIR_PATH}/${BISON_OUTPUT_FILENAME})
set(ALPHA_PARSER_HEADER_FILEPATH ${AUTOGEN_INCLUDE_DIR_PATH}/${BISON_HEADER_FILENAME})

BISON_TARGET(ALPHA_PARSER
        ${CMAKE_CURRENT_SOURCE_DIR}/${BISON_SPEC_FILENAME}
        ${ALPHA_PARSER_OUTPUT_FILEPATH}
        DEFINES_FILE ${ALPHA_PARSER_HEADER_FILEPATH}
)

# We create a phony target so we force BISON files to be generated before compilation.
add_custom_target(generate_parser ALL
        DEPENDS ${BISON_ALPHA_PARSER_OUTPUTS}
)

add_library(alpha_parser STATIC ${ALPHA_PARSER_OUTPUT_FILEPATH})

target_include_directories(alpha_parser PRIVATE
        ${AUTOGEN_INCLUDE_DIR_PATH}
        ${CMAKE_SOURCE_DIR}/include/core
)
