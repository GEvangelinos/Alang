if (NOT Python3_FOUND)
    message(WARNING "Could not find Python3. Please install `Python3`.")
endif ()

# Define generated diagnostics paths
get_filename_component(CURRENT_COMPONENT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(GENERATED_INCLUDE_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/include")
set(GENERATED_INCLUDE_COMPONENT_DIR "${GENERATED_INCLUDE_DIR}/${CURRENT_COMPONENT_NAME}")
set(GENERATED_SRC_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/src")
make_dir_verbose("${GENERATED_INCLUDE_DIR}")
make_dir_verbose("${GENERATED_INCLUDE_COMPONENT_DIR}")
make_dir_verbose("${GENERATED_SRC_DIR}")

# Extract Parser's basename and extension (typically .y):
get_filename_component(parser_basename ${BISON_SPEC_FILENAME} NAME_WE)
get_filename_component(parser_extension ${BISON_SPEC_FILENAME} EXT)

# Create the filename for the injected parser:
set(logged_file_marker "_LOGGED_")
set(LOG_INJECTED_BISON_SPEC_FILENAME "${parser_basename}${logged_file_marker}${parser_extension}")

set(FULLPATH_OF_IR_OPCODE_SPEC
    "${CMAKE_CURRENT_SOURCE_DIR}/config/${IR_OPCODE_SPEC_FILENAME}")

set(FULLPATH_OF_IR_OPCODE_HEADER
    "${GENERATED_INCLUDE_COMPONENT_DIR}/${IR_OPCODE_HEADER_FILENAME}")
set(FULLPATH_OF_IR_OPCODE_SOURCE
    "${GENERATED_SRC_DIR}/${IR_OPCODE_SOURCE_FILENAME}")
set(FULLPATH_OF_IR_OPCODE_INFO_TRAITS_HEADER
    "${GENERATED_INCLUDE_COMPONENT_DIR}/${IR_OPCODE_INFO_TRAITS_HEADER_FILENAME}")
set(FULLPATH_OF_IR_OPCODE_OPT_TRAITS_HEADER
    "${GENERATED_INCLUDE_COMPONENT_DIR}/${IR_OPCODE_OPT_TRAITS_HEADER_FILENAME}")

set(GENERATED_IR_OPCODE_HEADERS
    ${FULLPATH_OF_IR_OPCODE_HEADER}
    ${FULLPATH_OF_IR_OPCODE_INFO_TRAITS_HEADER}
    ${FULLPATH_OF_IR_OPCODE_OPT_TRAITS_HEADER}
)
set(GENERATED_IR_OPCODE_SOURCES
    ${FULLPATH_OF_IR_OPCODE_SOURCE}
)

if (NOT HATE_PYTHON_MODE)
    file(GLOB injector_files scripts/log_injector/*.py)
    add_custom_command(
        OUTPUT ${GENERATED_SRC_DIR}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        COMMAND ${Python3_EXECUTABLE} -B "${CMAKE_CURRENT_SOURCE_DIR}/scripts/log_injector/main.py" # -B is used to disable .pyc (__pycache__)
        --input ${CMAKE_CURRENT_SOURCE_DIR}/src/${BISON_SPEC_FILENAME}
        --output ${GENERATED_SRC_DIR}/${LOG_INJECTED_BISON_SPEC_FILENAME}
        --log-fn display_trace
        DEPENDS
        ${injector_files}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${BISON_SPEC_FILENAME}"
        COMMENT "[PYTHON][LOG_INJECTOR] Generating log injected parser file"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        VERBATIM
    )

    file(GLOB ir_operations_generator_files scripts/ir_opcode_generator/*.py)

    add_custom_command(
        OUTPUT
        ${GENERATED_IR_OPCODE_SOURCES}
        BYPRODUCTS
        ${GENERATED_IR_OPCODE_HEADERS}
        COMMAND
        ${Python3_EXECUTABLE}
        -B "${CMAKE_CURRENT_SOURCE_DIR}/scripts/ir_opcode_generator/main.py" # -B is used to disable .pyc (__pycache__)
        --in-ir-spec ${FULLPATH_OF_IR_OPCODE_SPEC}
        --out-iropcode-header ${FULLPATH_OF_IR_OPCODE_HEADER}
        --out-iropcode-source ${FULLPATH_OF_IR_OPCODE_SOURCE}
        --out-iropcode-info-traits-header ${FULLPATH_OF_IR_OPCODE_INFO_TRAITS_HEADER}
        --out-iropcode-opt-traits-header ${FULLPATH_OF_IR_OPCODE_OPT_TRAITS_HEADER}
        DEPENDS
        ${ir_operations_generator_files}
        "${CMAKE_CURRENT_SOURCE_DIR}/config/${IR_OPCODE_SPEC_FILENAME}"
        COMMENT "[PYTHON][IR_OPCODE] Generating ir opcodes and traits header files"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        VERBATIM
    )
endif ()

find_package(BISON QUIET)
if (BISON_FOUND)
    log_info("Found BISON: ${BISON_EXECUTABLE} (found version ${BISON_VERSION})")
else ()
    message(FATAL_ERROR "Could not find BISON. Please install `bison`.")
endif ()

set(ALPHA_PARSER_OUTPUT_FILEPATH "${GENERATED_SRC_DIR}/${BISON_OUTPUT_FILENAME}")
set(ALPHA_PARSER_HEADER_FILEPATH "${GENERATED_INCLUDE_COMPONENT_DIR}/${BISON_HEADER_FILENAME}")

if (NOT HATE_PYTHON_MODE)
endif ()

# We create a phony target so we force BISON files to be generated before compilation.
if (NOT HATE_PYTHON_MODE)
    bison_target(ALPHA_PARSER
        "${GENERATED_SRC_DIR}/${LOG_INJECTED_BISON_SPEC_FILENAME}"
        "${ALPHA_PARSER_OUTPUT_FILEPATH}"
        DEFINES_FILE ${ALPHA_PARSER_HEADER_FILEPATH}
    )
    add_custom_target(generate_parser DEPENDS
        "${BISON_ALPHA_PARSER_OUTPUTS}"
        "${GENERATED_SRC_DIR}/${LOG_INJECTED_BISON_SPEC_FILENAME}"
    )
else () # This is the case exists for when python is not allowed
    bison_target(ALPHA_PARSER
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${BISON_SPEC_FILENAME}"
        "${ALPHA_PARSER_OUTPUT_FILEPATH}"
        DEFINES_FILE "${ALPHA_PARSER_HEADER_FILEPATH}"
    )
    add_custom_target(generate_parser  DEPENDS
        "${BISON_ALPHA_PARSER_OUTPUTS}"
        "src/${BISON_SPEC_FILENAME}"
    )
endif ()

add_library(alpha_parser_api INTERFACE)
target_include_directories(alpha_parser_api INTERFACE
    "${GENERATED_INCLUDE_DIR}"
    "${GENERATED_INCLUDE_DIR}/parser"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/parser"
)
target_link_libraries(alpha_parser_api INTERFACE
    support_api
    alpha_settings_api
)

#TODO short in alphabetical order! (like in directory explorer tab)
add_library(alpha_parser_impl STATIC
    ${ALPHA_PARSER_OUTPUT_FILEPATH} # <- Generated
    src/symbol_table.cpp
    src/parser_context.cpp
    src/ir_expr.cpp
    src/L1_driver/semantic_system.cpp
    src/L1_driver/semantic_system_support.cpp
    src/L2_builders/lvalue_resolver.cpp
    src/L2_builders/control_flow_manager.cpp
    src/L2_builders/expr_builders.cpp
    src/L2_builders/semantic_subsystem.cpp
    src/L2_builders/block_manager.cpp
    src/L3_ir_infra/expr_optimizer.cpp
    src/L3_ir_infra/expr_maker.cpp
    src/L3_ir_infra/quad_handler.cpp
    invariants/ir_opcode_static_invariants.cpp
)

# To silence false positive of Bison's generated code.
set_source_files_properties(${ALPHA_PARSER_OUTPUT_FILEPATH} PROPERTIES COMPILE_OPTIONS -Wno-free-nonheap-object)
set_source_files_properties(${ALPHA_PARSER_OUTPUT_FILEPATH} PROPERTIES COMPILE_OPTIONS -Wno-unused-but-set-variable)

target_include_directories(alpha_parser_impl PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${GENERATED_INCLUDE_COMPONENT_DIR}"  # For Bison-generated .cpp to find its corresponding header
)
target_link_libraries(alpha_parser_impl PRIVATE
    alpha_parser_api
    alpha_core_api
    alpha_diagnostics_api
    alpha_scanner_api
)

add_dependencies(alpha_parser_impl generate_parser_scanner)

if (NOT HATE_PYTHON_MODE)
    target_sources(alpha_parser_impl PRIVATE ${GENERATED_IR_OPCODE_SOURCES})
endif ()
