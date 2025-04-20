add_library(fmt STATIC format.cpp)
target_include_directories(fmt PRIVATE ${CMAKE_SOURCE_DIR}/include)
