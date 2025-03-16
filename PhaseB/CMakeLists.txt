cmake_minimum_required(VERSION 3.18)
project(Alpha VERSION 0.2 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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
set(BISON_SPEC_FILENAME "alphaParser.l") # <-- Modifying .y filename requires manual change.


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
