add_subdirectory(core)

add_executable(alphaParserDriver.out alphaParserDriver.cpp)

target_link_libraries(alphaParserDriver.out PUBLIC alpha_core)