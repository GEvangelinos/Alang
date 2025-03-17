add_subdirectory(core)
add_subdirectory(parser)
add_subdirectory(scanner)

add_dependencies(alpha_scanner alpha_parser)

add_executable(${ALPHA_DRIVER_EXECUTABLE_NAME} alphaDriver.cpp)

target_link_libraries(${ALPHA_DRIVER_EXECUTABLE_NAME} PUBLIC alpha_core)
target_link_libraries(${ALPHA_DRIVER_EXECUTABLE_NAME} PUBLIC alpha_parser)
target_link_libraries(${ALPHA_DRIVER_EXECUTABLE_NAME} PUBLIC alpha_scanner)


target_include_directories(${ALPHA_DRIVER_EXECUTABLE_NAME} PRIVATE
        ${AUTOGEN_INCLUDE_DIR_PATH}
        ${CMAKE_SOURCE_DIR}/include/core
)
