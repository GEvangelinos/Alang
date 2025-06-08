if (NOT Python3_FOUND)
    message(WARNING "Could not find Python3. Please install `Python3`.")
endif ()

file(GLOB injector_files scripts/log_injector/*.py)

# Extract Parser's basename and extension (typically .y):
get_filename_component(parser_basename ${BISON_SPEC_FILENAME} NAME_WE)
get_filename_component(parser_extension ${BISON_SPEC_FILENAME} EXT)

# Create the filename for the injected parser: 
set(logged_file_marker "_LOGGED_")
set(LOG_INJECTED_BISON_SPEC_FILENAME "${parser_basename}${logged_file_marker}${parser_extension}")

if (NOT HATE_PYTHON_MODE)
    add_custom_command(
        OUTPUT ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        COMMAND ${Python3_EXECUTABLE} -B "${CMAKE_CURRENT_SOURCE_DIR}/scripts/log_injector/main.py" # -B is used to disable .pyc (__pycache)
        --input ${CMAKE_CURRENT_SOURCE_DIR}/src/${BISON_SPEC_FILENAME}
        --output ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        --log-fn display_trace
        DEPENDS ${injector_files} src/${BISON_SPEC_FILENAME}
        COMMENT "[PYTHON][LOG_INJECTOR] Generating log injected parser file"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        VERBATIM
    )
endif ()

find_package(BISON QUIET)
if (BISON_FOUND)
    message(STATUS "Found BISON: ${BISON_EXECUTABLE} (found version ${BISON_VERSION})")
else ()
    message(FATAL_ERROR "Could not find BISON. Please install `bison`.")
endif ()

set(ALPHA_PARSER_OUTPUT_FILEPATH ${AUTOGEN_SRC_DIR_PATH}/${BISON_OUTPUT_FILENAME})
set(ALPHA_PARSER_HEADER_FILEPATH ${AUTOGEN_INCLUDE_DIR_PATH}/${BISON_HEADER_FILENAME})

if (NOT HATE_PYTHON_MODE)
    BISON_TARGET(ALPHA_PARSER
        ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        ${ALPHA_PARSER_OUTPUT_FILEPATH}
        DEFINES_FILE ${ALPHA_PARSER_HEADER_FILEPATH}
    )
else () # This is the case exists for when python is not allowed
    BISON_TARGET(ALPHA_PARSER
        src/${BISON_SPEC_FILENAME}
        ${ALPHA_PARSER_OUTPUT_FILEPATH}
        DEFINES_FILE ${ALPHA_PARSER_HEADER_FILEPATH}
    )
endif ()

# We create a phony target so we force BISON files to be generated before compilation.
if (NOT HATE_PYTHON_MODE)
    add_custom_target(generate_parser ALL DEPENDS
        ${BISON_ALPHA_PARSER_OUTPUTS}
        ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
    )
else () # This is the case exists for when python is not allowed
    add_custom_target(generate_parser ALL DEPENDS
        ${BISON_ALPHA_PARSER_OUTPUTS}
        src/${BISON_SPEC_FILENAME}
    )
endif ()

add_library(alpha_parser STATIC
    ${ALPHA_PARSER_OUTPUT_FILEPATH}
    src/symbol_table.cpp
    src/RELOCATE_backpatcher.cpp
    src/L1_driver/semantic_driver.cpp
    src/L1_driver/semantic_driver_services.cpp
)

target_include_directories(alpha_parser PUBLIC
    "${AUTOGEN_INCLUDE_DIR_PATH}"
    "include"
    "include/parser"
    PRIVATE
    "src"
)

target_link_libraries(alpha_parser PUBLIC
    alpha_core
    alpha_scanner
    utils
)
