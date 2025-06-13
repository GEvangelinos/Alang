# NOTE: AUTOMATIC FORMATTING IS DISABLED FOR THIS FILE (TIRED OF FIGHTING THE IDE).
# REMOVE THE FOLLOWING LINES TO RE-ENABLE FORMATTING:
# @formatter:off
# clang-format off

# ---------------------------------------------------------------------------
# Check Python dependency for the diagnostics generator
# ---------------------------------------------------------------------------
if (NOT Python3_FOUND)
    message(FATAL_ERROR
        "Python3 is required for the diagnostics system to be auto-generated.\n"
        "Please install Python3 and ensure it is discoverable by CMake."
    )
endif ()

# ---------------------------------------------------------------------------
# Setup paths for generated diagnostics (headers + source)
# ---------------------------------------------------------------------------
get_filename_component(CURRENT_COMPONENT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(GENERATED_INCLUDE_DIR           "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/include")
set(GENERATED_INCLUDE_COMPONENT_DIR "${GENERATED_INCLUDE_DIR}/${CURRENT_COMPONENT_NAME}")
set(GENERATED_SRC_DIR               "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/src")

make_dir_verbose("${GENERATED_INCLUDE_DIR}")
make_dir_verbose("${GENERATED_INCLUDE_COMPONENT_DIR}")
make_dir_verbose("${GENERATED_SRC_DIR}")

# ---------------------------------------------------------------------------
# Configure diagnostics generator script
# ---------------------------------------------------------------------------
set(DIAGNOSTIC_GENERATOR      "${CMAKE_CURRENT_SOURCE_DIR}/scripts/diagnostic_generator/main.py")
set(DIAGNOSTICS_CONFIG_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/config")
set(DIAGNOSTICS_SPEC_FILEPATH "${DIAGNOSTICS_CONFIG_DIR}/${DIAGNOSTICS_SPEC_FILENAME}")

# The diagnostics generator produces many output files, and more may be added in the future.
# To avoid listing them individually (or relying on just one), we use a dummy "stamp" file
# as the declared output. This allows CMake to track the generator dependencies accurately
# and avoid re-running the script unless something actually changes.
set(DIAGNOSTICS_STAMP_FILE    "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/.gen.stamp")

file(GLOB_RECURSE diagnostic_generator_files "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*")
add_custom_command(
    OUTPUT            "${DIAGNOSTICS_STAMP_FILE}"
    COMMAND           "${Python3_EXECUTABLE}" -B "${DIAGNOSTIC_GENERATOR}"
    --spec-filepath   "${DIAGNOSTICS_CONFIG_DIR}/${DIAGNOSTICS_SPEC_FILENAME}"
    --out-include-dir "${GENERATED_INCLUDE_COMPONENT_DIR}"
    --out-src-dir     "${GENERATED_SRC_DIR}"
    --file-prefix     "diagnostics"
    --stamp-filepath  "${DIAGNOSTICS_STAMP_FILE}"
    DEPENDS
         ${diagnostic_generator_files}
        "${DIAGNOSTICS_SPEC_FILEPATH}"
    COMMENT "[PYTHON][DIAGNOSTICS] Generating diagnostic headers and source files"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_target(generate_diagnostics DEPENDS "${DIAGNOSTICS_STAMP_FILE}")

# ---------------------------------------------------------------------------
# Diagnostics API (header interface)
# ---------------------------------------------------------------------------
add_library(alpha_diagnostics_api INTERFACE)
add_dependencies(alpha_diagnostics_api generate_diagnostics)

target_include_directories(alpha_diagnostics_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${GENERATED_INCLUDE_DIR}"
)
target_link_libraries(alpha_diagnostics_api INTERFACE alpha_core_api)

# ---------------------------------------------------------------------------
# Diagnostics Implementation (source + engine)
# ---------------------------------------------------------------------------
add_library(alpha_diagnostics_impl STATIC
    "src/diagnostic_engine.cpp"
)
target_link_libraries(alpha_diagnostics_impl PRIVATE
    alpha_diagnostics_api
    alpha_core_api
)
