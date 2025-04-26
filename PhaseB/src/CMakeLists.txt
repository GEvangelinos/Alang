add_subdirectory(arguinator)
add_subdirectory(core)
add_subdirectory(parser)
add_subdirectory(scanner)
add_subdirectory(third_party)

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
add_dependencies(${ALPHA_DRIVER_EXECUTABLE}
        generate_scanner
        generate_parser)

target_link_libraries(${ALPHA_DRIVER_EXECUTABLE}
        PRIVATE
        arguinator
        alpha_core
        alpha_parser
        alpha_scanner)


target_include_directories(${ALPHA_DRIVER_EXECUTABLE} PRIVATE
        "${AUTOGEN_INCLUDE_DIR_PATH}"
        "${CMAKE_SOURCE_DIR}/include"
)

find_package(Python3 QUIET)
if(Python3_FOUND)
        message(STATUS "Found Python3: ${Python3_EXECUTABLE} (found version ${Python3_VERSION})")
else()
        message(WARNING "Could not find Python3. Please install `Python3`.")
endif()

file(GLOB WORKING_TEST_FILES "${WORKING_TESTS_DIR}/*")
file(GLOB ERROR_TEST_FILES "${ERROR_TESTS_DIR}/*")

set(ALL_TEST_FILES ${WORKING_TEST_FILES})
list(APPEND ALL_TEST_FILES ${ERROR_TEST_FILES})
set(SYMBOL_TABLE_EXPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/SYMBOL_TABLE_EXPORTS")

add_custom_target(run_all_validators)

foreach(test_file ${ALL_TEST_FILES})
        get_filename_component(test_file_name ${test_file} NAME)
        set(GOLDEN_SYMBOL_TABLE_CSV "GOLD_${test_file_name}.st.csv")
        set(SYMBOL_TABLE_CSV "${test_file_name}.st.csv")
        set(GOLDEN_COMPILE_ERRORS_CSV "GOLD_${test_file_name}.error.csv")
        set(COMPILE_ERRORS_CSV "${test_file_name}.error.csv")

        add_custom_command(
                OUTPUT ${SYMBOL_TABLE_CSV} ${COMPILE_ERRORS_CSV}
                COMMAND "$<TARGET_FILE:${ALPHA_DRIVER_EXECUTABLE}>"
                --input-file "${test_file}"
                --export-symbol-table
                --export-compile-errors
                --no-show-errors
                DEPENDS ${ALPHA_DRIVER_EXECUTABLE}
                VERBATIM
        )

        add_custom_target(
                "run_csv_${test_file_name}"
                DEPENDS "${SYMBOL_TABLE_CSV}"
                VERBATIM
        )

        add_dependencies("run_csv_${test_file_name}" "${ALPHA_DRIVER_EXECUTABLE}")

        add_custom_target("run_validator_${test_file_name}"
                COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/scripts/export_validator.py"
                --golden-file "${GOLDEN_SYMBOL_TABLE_DIR}/${GOLDEN_SYMBOL_TABLE_CSV}"
                --export-file "${SYMBOL_TABLE_EXPORT_DIR}/${SYMBOL_TABLE_CSV}"
                DEPENDS "run_csv_${test_file_name}"
                VERBATIM
        )

        add_dependencies(run_all_validators run_validator_${test_file_name})
endforeach()
