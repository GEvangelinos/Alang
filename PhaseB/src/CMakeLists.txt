add_subdirectory(core)
add_subdirectory(parser)
add_subdirectory(scanner)

add_executable(${ALPHA_DRIVER_EXECUTABLE_NAME} alphaParserDriver.cpp)

target_link_libraries(${ALPHA_DRIVER_EXECUTABLE_NAME} PUBLIC alpha_core)
target_link_libraries(${ALPHA_DRIVER_EXECUTABLE_NAME} PUBLIC alpha_parser)
target_link_libraries(${ALPHA_DRIVER_EXECUTABLE_NAME} PUBLIC alpha_scanner)
