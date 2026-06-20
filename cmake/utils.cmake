function(add_compiler_test TEST_NAME)
    set(SOURCE_FILE "${TEST_NAME}.asc")
    set(ABC_FILE "${CMAKE_BINARY_DIR}/ABC_BINARIES/${TEST_NAME}.abc")

    # Step 1: Compile
    add_custom_command(
        OUTPUT ${ABC_FILE}
        COMMAND ${CMAKE_BINARY_DIR}/executables/alang --source ${CMAKE_SOURCE_DIR}/.tests/${SOURCE_FILE} --show_abc
        DEPENDS ${CMAKE_SOURCE_DIR}/.tests/${SOURCE_FILE} alang
        COMMENT "Compiling ${SOURCE_FILE} to ${ABC_FILE}"
    )

    # Step 2: Run VM
    add_custom_target(run_${TEST_NAME}
        COMMAND executables/avm --source ${ABC_FILE}
        DEPENDS ${ABC_FILE} avm
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
    )
endfunction()