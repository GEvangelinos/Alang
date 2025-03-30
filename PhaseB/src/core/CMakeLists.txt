add_library(alpha_core STATIC
        errorTracker.cpp
        symbolTable.cpp
)

target_include_directories(alpha_core PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
