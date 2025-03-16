set(FLEX_SPEC_FILENAME "alphaScanner.l")
set(FLEX_OUTPUT_FILENAME "alphaScanner.cpp")
set(FLEX_HEADER_FILENAME "alphaScanner.hpp")

find_package(FLEX QUIET)
if(FLEX_FOUND)
        message(STATUS "Found FLEX: ${FLEX_EXECUTABLE} (found version ${FLEX_VERSION})")
else()
        message(FATAL_ERROR "Could not find FLEX. Please install `flex`.")
endif()

FLEX_TARGET(ALPHA_SCANNER
        ${CMAKE_CURRENT_SOURCE_DIR}/${FLEX_SPEC_FILENAME}
        ${AUTOGEN_SRC_DIR_PATH}/${FLEX_OUTPUT_FILENAME}
        COMPILE_FLAGS "--header-file=${AUTOGEN_INCLUDE_DIR_PATH}/${FLEX_HEADER_FILENAME}"
)

add_library(alpha )