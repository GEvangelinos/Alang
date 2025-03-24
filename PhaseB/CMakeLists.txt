cmake_minimum_required(VERSION 3.18)
project(Alpha VERSION 0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect and set the preferred C++ compiler
find_program(CMAKE_CXX_COMPILER_CLANG NAMES clang++)
find_program(CMAKE_CXX_COMPILER_GCC NAMES g++)

if (CMAKE_CXX_COMPILER_CLANG)
    set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_CLANG}" CACHE STRING "Path to Clang C++ compiler" FORCE)
    message(STATUS "Using Clang: ${CMAKE_CXX_COMPILER_CLANG}")
elseif (CMAKE_CXX_COMPILER_GCC)
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
                message(STATUS "CMAKE: include-what-you-use enabled.")
        else()
                message(SEND_ERROR "CMAKE: include-what-you-use not found on system.")
        endif()
else()
        message(STATUS "CMAKE: Include-What-You-Use disabled.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/exercutables")

##########################################--==>> CONFIGURABLE <<==--###########################################
###############################################################################################################
set(ALPHA_DRIVER_EXECUTABLE_NAME "alphaDriver.out") # <- Name of the executable (Syntax analyzer)             #
set(AUTOGEN_ROOT_DIR_NAME "autogen") # <- Name of the directory where the auto-generated files are stored.    #
set(FLEX_OUTPUT_FILENAME "alphaScanner.cpp") # <-- .cpp filename of the auto-generated scanner's source file. #
set(FLEX_HEADER_FILENAME "alphaScanner.hpp") # <-- .hpp filename of the auto-generated scanner's header file. #
set(BISON_OUTPUT_FILENAME "alphaParser.cpp") # <-- .cpp filename of the auto-generated parser's source file.  #
set(BISON_HEADER_FILENAME "alphaParser.hpp") # <-- .hpp filename of the auto-generated parser's header file.  #
###############################################################################################################

set(FLEX_SPEC_FILENAME "alphaScanner.l") # <--  Modifying .l filename requires manual change.
set(BISON_SPEC_FILENAME "alphaParser.y") # <-- Modifying .y filename requires manual change.


# Create directory that hosts code from flex and bison
set(AUTOGEN_ROOT_DIR_PATH ${CMAKE_SOURCE_DIR}/autogen CACHE PATH "Path to the root directory for auto-generated files")
set(AUTOGEN_SRC_DIR_PATH ${AUTOGEN_ROOT_DIR_PATH}/src CACHE PATH "Path to the directory for auto-generated source files")
set(AUTOGEN_INCLUDE_DIR_PATH ${AUTOGEN_ROOT_DIR_PATH}/include CACHE PATH "Path to the directory for auto-generated header files")
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

# This property tells CMake to delete everything in ${AUTOGEN_ROOT_DIR_PATH} when a "make clean" is run.
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES ${AUTOGEN_ROOT_DIR_PATH})
