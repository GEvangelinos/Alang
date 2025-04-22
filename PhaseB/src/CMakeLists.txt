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
        driver.cpp
        app_main.cpp
)
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

# add_custom_target(mycustomtarget
#         COMMAND $<TARGET_FILE:${ALPHA_DRIVER_EXECUTABLE}>)

# add_dependencies(mycustomtarget ${ALPHA_DRIVER_EXECUTABLE})



foreach(test ${WORKING_TESTS})
        set(INPUT_FILE "${ALPHA_DRIVER_WORKING_TESTS_DIR}/${test}")
        set(GOLDEN_CSV "${ALPHA_DRIVER_GOLDEN_DIR}/STG_${test}.csv")
        set(EXPORT_CSV "${ALPHA_DRIVER_EXPORT_DIR}/STE_${test}.csv")

        add_custom_command(
                OUTPUT "${EXPORT_CSV}"
                COMMAND "$<TARGET_FILE:${ALPHA_DRIVER_EXECUTABLE}>"
                --input-file "${INPUT_FILE}"
                --export-symtable "${ALPHA_DRIVER_EXPORT_DIR}" "${EXPORT_CSV}"
                --quiet
                DEPENDS ${ALPHA_DRIVER_EXECUTABLE} "$<TARGET_FILE:${ALPHA_DRIVER_EXECUTABLE}>" "${INPUT_FILE}"
        )



        add_custom_target(
                run_csv_${test}
                DEPENDS "${EXPORT_CSV}"
        )

        add_dependencies(run_csv_${test} ${ALPHA_DRIVER_EXECUTABLE})
        find_package(Python3 QUIET)
        if(Python3_FOUND)
                message(STATUS "Found Python3: ${Python3_EXECUTABLE} (found version ${Python3_VERSION})")
        else()
                message(WARNING "Could not find Python3. Please install `Python3`.")
        endif()

        message(STATUS "INPUT_FILE = " ${INPUT_FILE})
        message(STATUS "GOLDEN_CSV = " ${GOLDEN_CSV})
        message(STATUS "EXPORT_CSV = " ${EXPORT_CSV})

        add_custom_target(run_validator_${test}
                COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/scripts/export_validator.py"
                "--golden-file"
                "${GOLDEN_CSV}"
                "--export-file"
                "${EXPORT_CSV}"
                DEPENDS "run_csv_${test}"
        )
endforeach()
