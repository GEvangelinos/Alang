cmake_minimum_required(VERSION 3.18)
project(Alpha VERSION 1.0 LANGUAGES CXX)

# === Configurable build options ===
option(ENABLE_IWYU "Enable include-what-you-use integration" OFF)
option(DEBUG_MODE "DEBUG_MODE: enables extra sanity checking inside the code" OFF)
option(DEBUG_MODE "OPTIMIZED_MODE: enables some C++ code optimizations, disable parser trace" OFF)
option(COMPILER_SUPPORTS_STD_FORMAT "Used by the format adapter to switch from std:: to fmt::" OFF)
set(DEBUG_MODE ON) # TODO unset when done, I did this so I dont forget it on testing.
set(COMPILER_SUPPORTS_STD_FORMAT "Used by the format adapter to switch from std:: to fmt::" OFF)

##########################################--==>> CONFIGURABLE <<==--###########################################
###############################################################################################################
set(ALPHA_DRIVER_EXECUTABLE alpha_driver.out CACHE STRING "Name of the main executable (e.g., the syntax analyzer)")
set(AUTOGEN_ROOT_DIR "autogen" CACHE STRING "Name of the directory where auto-generated files are stored")
set(FLEX_OUTPUT_FILENAME "alpha_scanner.cpp" CACHE STRING "Filename of the Flex-generated C++ source file")
set(FLEX_HEADER_FILENAME "alpha_scanner.hpp" CACHE STRING "Filename of the Flex-generated header file")
set(BISON_OUTPUT_FILENAME "alpha_parser.cpp" CACHE STRING "Filename of the Bison-generated C++ source file")
set(BISON_HEADER_FILENAME "alpha_parser.hpp" CACHE STRING "Filename of the Bison-generated header file")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/executables" CACHE PATH "Directory where compiled executables will be placed")
###############################################################################################################

set(FLEX_SPEC_FILENAME "alpha_scanner_spec.l") # <-- Modifying .l filename requires manual change.
set(BISON_SPEC_FILENAME "alpha_parser_spec.y") # <-- Modifying .y filename requires manual change.


##########################################--==>> TESTS <<==--###########################################
set(SYMBOL_TABLE_WORKING_TESTS_DIR "${CMAKE_SOURCE_DIR}/tests/working")
set(SYMBOL_TABLE_ERROR_TESTS_DIR "${CMAKE_SOURCE_DIR}/tests/error")
set(SYMBOL_TABLE_GOLDEN_DIR "${CMAKE_SOURCE_DIR}/tests/symtable_golden_exports")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect and set the preferred C++ compiler
find_program(CMAKE_CXX_COMPILER_CLANG NAMES clang++)
find_program(CMAKE_CXX_COMPILER_GCC NAMES g++)

if(CMAKE_CXX_COMPILER_CLANG)
        set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_CLANG}" CACHE STRING "Path to Clang C++ compiler" FORCE)
        message(STATUS "Using Clang: ${CMAKE_CXX_COMPILER_CLANG}")
elseif(CMAKE_CXX_COMPILER_GCC)
        set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_GCC}" CACHE STRING "Path to GCC C++ compiler" FORCE)
        message(WARNING "Clang not found. Falling back to GCC: ${CMAKE_CXX_COMPILER_GCC}")
else()
        message(FATAL_ERROR "No suitable C++ compiler found. Please install either Clang or GCC.")
endif()

# General warnings
add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
)

# Reuires to manually define ENABLE_IWYU at the configuration process (cmake -DENABLE_IWYU=ON)
if(ENABLE_IWYU)
        find_program(IWYU_PATH NAMES include-what-you-use)
        if(IWYU_PATH)
                set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE include-what-you-use --no-fwd-decls)
                message(WARNING "CMAKE: include-what-you-use enabled.")
        else()
                message(SEND_ERROR "CMAKE: include-what-you-use not found on system.")
        endif()
endif()

if(DEBUG_MODE)
        add_compile_options(-g)
        add_compile_definitions(DEBUG_MODE)
        message(WARNING "CMAKE: Running with DEBUG_MODE ON")
endif()

if(COMPILER_SUPPORTS_STD_FORMAT)
        message(WARNING "CMAKE: IMPLEMNT COMPILER_SUPPORT_STD_FORMAT")
endif()


if(OPTIMIZED_MODE)
        add_compile_definitions(OPTIMIZED_MODE)
        message(WARNING "CMAKE: Running with OPTIMIZED_MODE ON")
endif()

# Create directory that hosts code from flex and bison
set(AUTOGEN_ROOT_DIR_PATH "${CMAKE_BINARY_DIR}/autogen" CACHE PATH "Path to the root directory for auto-generated files")
set(AUTOGEN_SRC_DIR_PATH "${AUTOGEN_ROOT_DIR_PATH}/src" CACHE PATH "Path to the directory for auto-generated source files")
set(AUTOGEN_INCLUDE_DIR_PATH "${AUTOGEN_ROOT_DIR_PATH}/include" CACHE PATH "Path to the directory for auto-generated header files")
set(AUTOGEN_DIR_LIST
        ${AUTOGEN_ROOT_DIR_PATH}
        ${AUTOGEN_SRC_DIR_PATH}
        ${AUTOGEN_INCLUDE_DIR_PATH}
)

foreach(DIR ${AUTOGEN_DIR_LIST})
        file(MAKE_DIRECTORY ${DIR})
        if(EXISTS ${DIR} AND IS_DIRECTORY ${DIR})
                message(STATUS "Directory created successfully: ${DIR}")
        else()
                message(FATAL_ERROR "Failed to create directory: ${DIR}")
        endif()
endforeach()

add_subdirectory(src)

# This property tells CMake to delete ${AUTOGEN_ROOT_DIR_PATH} and its contents when a "make clean" is run.
# set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES ${AUTOGEN_ROOT_DIR_PATH})

# Find all files stored in `${AUTOGEN_ROOT_DIR_PATH}` and delete them (but keep the directory structure intact).
file(GLOB_RECURSE AUTOGEN_FILES ${AUTOGEN_ROOT_DIR_PATH})
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${AUTOGEN_FILES}")
