add_library(arguinator STATIC arguinator.cpp)

target_include_directories(arguinator PRIVATE "${CMAKE_SOURCE_DIR}/include")
