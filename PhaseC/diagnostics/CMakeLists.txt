if (NOT Python3_FOUND)
    message(FATAL_ERROR
        "Could not find Python3. Please install Python3.\n"
        "Python is required for the diagnostic system to be auto-generated."
    )
endif ()

get_filename_component(CURRENT_COMPONENT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME) # Used to generate "include" dir
# Define generated diagnostics paths
set(GENERATED_DIAGNOSTICS_INCLUDE_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/include/${CURRENT_COMPONENT_NAME}")
set(GENERATED_DIAGNOSTICS_SRC_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/src")

# Define path to the diagnostic generator script and config directory
set(DIAGNOSTIC_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/scripts/diagnostic_generator/main.py")
set(DIAGNOSTICS_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")

# Creating directories that will host the auto-generated files.
file(MAKE_DIRECTORY "${GENERATED_DIAGNOSTICS_INCLUDE_DIR}")
file(MAKE_DIRECTORY "${GENERATED_DIAGNOSTICS_SRC_DIR}")


add_custom_target(generate_diagnostics
    COMMAND "${Python3_EXECUTABLE}" "-B" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/diagnostic_generator/main.py" # -B is used to disable .pyc (__pycache__)
    "--input" "${DIAGNOSTICS_CONFIG_DIR}/${DIAGNOSTICS_SPEC_FILENAME}"
    "--out-include" "${GENERATED_DIAGNOSTICS_INCLUDE_DIR}"
    "--out-src" "${GENERATED_DIAGNOSTICS_SRC_DIR}"
    "--outfile-prefix" "diagnostics"
    COMMENT "[PYTHON][DIAGNOSTIC_GENERATOR] Generating diagnostic: {code, reporter}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_library(alpha_diagnostics_api INTERFACE)
add_dependencies(alpha_diagnostics_api generate_diagnostics)

target_include_directories(alpha_diagnostics_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${GENERATED_DIAGNOSTICS_INCLUDE_DIR}"
)
target_link_libraries(alpha_diagnostics_api INTERFACE alpha_core_api)

add_library(alpha_diagnostics_impl STATIC
    "src/diagnostic_engine.cpp"
)
target_link_libraries(alpha_diagnostics_impl PRIVATE
    alpha_diagnostics_api
    alpha_core_api
)

