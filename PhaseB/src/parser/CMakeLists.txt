find_package(Python3 QUIET)
if(Python3_FOUND)
        message(STATUS "Found Python3: ${Python3_EXECUTABLE} (found version ${Python3_VERSION})")
else()
        message(WARNING "Could not find Python3. Please install `Python3`.")
endif()

file(GLOB injector_files ${CMAKE_CURRENT_SOURCE_DIR}/log_injector/*.py)

# Extract Parser's basename and extension (typically .y):
get_filename_component(parser_basename ${BISON_SPEC_FILENAME} NAME_WE)
get_filename_component(parser_extension ${BISON_SPEC_FILENAME} EXT)

# Create the filename for the injected parser: 
set(LOG_INJECTED_BISON_SPEC_FILENAME "${parser_basename}_LOGGED${parser_extension}")

add_custom_command(
        OUTPUT ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        COMMAND ${Python3_EXECUTABLE} -B ${CMAKE_CURRENT_SOURCE_DIR}/log_injector/main.py # -B is used to disable .pyc (__pycache)
        --input ${CMAKE_CURRENT_SOURCE_DIR}/${BISON_SPEC_FILENAME}
        --output ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        --log-fn displayLog
        DEPENDS ${injector_files} ${CMAKE_CURRENT_SOURCE_DIR}/${BISON_SPEC_FILENAME}
        COMMENT "Injecting logging in bison parser"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} 
        VERBATIM
)


find_package(BISON QUIET)
if(BISON_FOUND)
        message(STATUS "Found BISON: ${BISON_EXECUTABLE} (found version ${BISON_VERSION})")
else()
        message(FATAL_ERROR "Could not find BISON. Please install `bison`.")
endif()

set(ALPHA_PARSER_OUTPUT_FILEPATH ${AUTOGEN_SRC_DIR_PATH}/${BISON_OUTPUT_FILENAME})
set(ALPHA_PARSER_HEADER_FILEPATH ${AUTOGEN_INCLUDE_DIR_PATH}/${BISON_HEADER_FILENAME})

BISON_TARGET(ALPHA_PARSER
        ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        ${ALPHA_PARSER_OUTPUT_FILEPATH}
        DEFINES_FILE ${ALPHA_PARSER_HEADER_FILEPATH}
)

# We create a phony target so we force BISON files to be generated before compilation.
add_custom_target(generate_parser ALL
        DEPENDS 
        ${BISON_ALPHA_PARSER_OUTPUTS}
        ${AUTOGEN_SRC_DIR_PATH}/${LOG_INJECTED_BISON_SPEC_FILENAME}
)

add_library(alpha_parser STATIC
        ${ALPHA_PARSER_OUTPUT_FILEPATH}
        semanticActions.cpp
)

target_include_directories(alpha_parser PRIVATE
        ${AUTOGEN_INCLUDE_DIR_PATH}
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/include/parser
)
