cmake_minimum_required(VERSION 3.18)

# Simple ANSI color helpers
string(ASCII 27 Esc)

# Reset & attributes
set(SGR_RESET "${Esc}[0m")
set(SGR_BOLD "${Esc}[1m")
set(SGR_UNDERLINE "${Esc}[4m")
set(SGR_BLINK "${Esc}[5m")

# Foreground colors
set(SGR_FG_BLACK "${Esc}[90m")
set(SGR_FG_RED "${Esc}[91m")
set(SGR_FG_GREEN "${Esc}[92m")
set(SGR_FG_YELLOW "${Esc}[93m")
set(SGR_FG_BLUE "${Esc}[94m")
set(SGR_FG_MAGENTA "${Esc}[95m")
set(SGR_FG_CYAN "${Esc}[96m")
set(SGR_FG_WHITE "${Esc}[97m")

# Set of logging function for displaying nice colorized messages.
function(log_ok message)
    message(STATUS ${SGR_BOLD}${SGR_FG_GREEN}${message}${SGR_RESET})
endfunction()

function(log_info message)
    message(STATUS ${SGR_BOLD}${SGR_FG_BLUE}${message}${SGR_RESET})
endfunction()

# Detect and set the preferred C++ compiler
find_program(CMAKE_CXX_COMPILER_CLANG NAMES clang++)
find_program(CMAKE_CXX_COMPILER_GCC NAMES g++)

# Configure the preferred C++ compiler before invoking project().
# CMake finalizes the compiler selection during the project() call.
if (CMAKE_CXX_COMPILER_CLANG)
    set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_CLANG}" CACHE STRING "Path to Clang C++ compiler" FORCE)
    log_info("Using Clang: ${CMAKE_CXX_COMPILER_CLANG}")
elseif (CMAKE_CXX_COMPILER_GCC)
    set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_GCC}" CACHE STRING "Path to GCC C++ compiler" FORCE)
    message(WARNING "Clang not found. Falling back to GCC: ${CMAKE_CXX_COMPILER_GCC}")
else ()
    message(FATAL_ERROR "No suitable C++ compiler found. Please install either Clang or GCC.")
endif ()

project(Alpha VERSION 1.2 LANGUAGES CXX)

find_package(Python3 QUIET)
if (Python3_FOUND AND NOT HATE_PYTHON_MODE)
    log_info("Found Python3: ${Python3_EXECUTABLE} (found version ${Python3_VERSION})")
else ()
    message(SEND_ERROR "PYTHON NOT FOUND!")
endif ()

find_program(VALGRIND_PATH NAMES valgrind)
if (VALGRIND_PATH)
    log_info("Found Valgrind: ${VALGRIND_PATH}")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Warnings applied to all builds
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wreorder
    -Wimplicit-fallthrough
)

# Errors applied to all builds
if (CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER_CLANG)
    add_compile_options(
        -Werror=missing-field-initializers
        -Werror=missing-designated-field-initializers
        -Werror=reorder-init-list
    )
endif ()

# Check if std::format is allowed
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
        #include <format>
        int main()
        {
                auto s = std::format(\"ZERO={}\", 0);
                return 0;
        }
" STD_FORMAT_SUPPORTED)

if (STD_FORMAT_SUPPORTED)
    log_info("Compilation test on std::format() succeeded. Adding compile time definition `" STD_FORMAT_SUPPORTED "`")
    add_compile_definitions(STD_FORMAT_SUPPORTED)
endif ()

if (DEBUG_MODE)
    message(WARNING "CMAKE: Running with `" DEBUG_MODE "` ON")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(WARNING "Enabling _GLIBCXX debugging")
        add_compile_definitions(
            _GLIBCXX_ASSERTIONS
            _GLIBCXX_DEBUG
            _GLIBCXX_DEBUG_PEDANTIC
        )
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(WARNING "Enabling _LIBCPP debugging")
        add_compile_definitions(
            _LIBCPP_DEBUG=2
            _LIBCPP_ENABLE_ASSERTIONS=1
            _LIBCPP_ENABLE_HARDENED_MODE=1
        )
    endif ()

    add_compile_options(
        -g
        #        -Wunused-result
        #        -Wnull-dereference
        #        -Wsuggest-override
        #        -Wlogical-op
        #        -Wuseless-cast
        #        -Wduplicated-cond
        #        -Wmissing-declarations
    )
    add_compile_definitions(DEBUG_MODE)
endif ()

if (CYA_MODE)
    message(WARNING "CMAKE: Running with `CYA_MODE` (Cover-Your-Ass Mode) ON")
    add_compile_definitions(CYA_MODE)
endif ()

if (OPTIMIZED_MODE)
    add_compile_options(
        -O3
        -march=native
    )
    add_compile_definitions(OPTIMIZED_MODE)
    log_info("CMAKE: Running with `" OPTIMIZED_MODE "` ON")
endif ()

if (ENABLE_IWYU)
    find_program(IWYU_PATH NAMES include-what-you-use)
    if (NOT IWYU_PATH)
        message(SEND_ERROR "CMAKE: include-what-you-use not found on system.")
    else ()
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
            "${IWYU_PATH}"
            "-Xiwyu" # Any IWYU flag must be provided after -Xiwyu flag!
            "--no_fwd_decls"
            "--update_comments"
        )
        message(WARNING "CMAKE: include-what-you-use enabled at ${IWYU_PATH}")
    endif ()
endif ()

if (HATE_PYTHON_MODE)
    add_compile_definitions(HATE_PYTHON_MODE)
    message(WARNING "Mode `HATE_PYTHON_MODE` has been activated...\n"
        "Pulling generator from cache. Python utilities are disabled"
    )

endif ()


# === Configurable build options ===
option(ENABLE_IWYU "Enable include-what-you-use integration" OFF)
option(DEBUG_MODE "DEBUG_MODE: enables extra sanity checking inside the code" OFF)
option(DEBUG_MODE "OPTIMIZED_MODE: enables some C++ code optimizations, disable parser trace" OFF)

function(dot_gen_applier [filename])
    get_filename_component(basename filename NAME_WLE)
    get_filename_component(extension filename NAME_WLE)


endfunction()

##########################################--==>> CONFIGURABLE <<==--###########################################
set(ALPHA_DRIVER_EXECUTABLE alang.out CACHE STRING "Name of the main executable (e.g., the syntax analyzer)")

set(AUTOGEN_DIRNAME "AUTOGEN" CACHE STRING "Name of the directory where auto-generated files are stored")

set(FLEX_OUTPUT_FILENAME "alpha_scanner.gen.cpp" CACHE STRING "Filename of the Flex-generated C++ source file")
set(FLEX_HEADER_FILENAME "alpha_scanner.gen.hpp" CACHE STRING "Filename of the Flex-generated header file")

set(BISON_OUTPUT_FILENAME "alpha_parser.gen.cpp" CACHE STRING "Filename of the Bison-generated C++ source file")
set(BISON_HEADER_FILENAME "alpha_parser.gen.hpp" CACHE STRING "Filename of the Bison-generated header file")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/executables" CACHE PATH "Directory where compiled executables will be placed")

set(IR_OPCODE_HEADER_FILENAME "ir_opcode.gen.hpp" CACHE STRING "Filename of the IR opcode header file")
set(IR_OPCODE_SOURCE_FILENAME "ir_opcode.gen.cpp" CACHE STRING "Filename of the IR opcode source file")
set(IR_OPCODE_INFO_TRAITS_HEADER_FILENAME "ir_opcode_info_traits.gen.hpp" CACHE STRING "Filename of the IR opcode info traits header file")
set(IR_OPCODE_OPT_TRAITS_HEADER_FILENAME "ir_opcode_opt_traits.gen.hpp" CACHE STRING "Filename of the IR opcode opt traits header file")
###############################################################################################################

set(FLEX_SPEC_FILENAME "alpha_scanner_spec.l")          # <-- Modifying filename requires manual change.
set(BISON_SPEC_FILENAME "alpha_parser_spec.y")          # <-- Modifying filename requires manual change.
set(DIAGNOSTICS_SPEC_FILENAME "diagnostics_spec.yaml")  # <-- Modifying filename requires manual change.
set(IR_OPCODE_SPEC_FILENAME "ir_opcode_spec.yaml")      # <-- Modifying filename requires manual change.

# Create directory that hosts code from CODE-GENERATORS (e.g. {log_injector, diagnostic_generator})
set(AUTOGEN_ROOT_DIR_PATH "${CMAKE_BINARY_DIR}/${AUTOGEN_DIRNAME}" CACHE PATH "Path to the root directory for auto-generated components")

##########################################--==>> TESTS <<==--###########################################
set(WORKING_TESTS_DIR "${CMAKE_SOURCE_DIR}/.tests/working")
set(ERROR_TESTS_DIR "${CMAKE_SOURCE_DIR}/.tests/error")
set(GOLDEN_SYMBOL_TABLES_DIR "${CMAKE_SOURCE_DIR}/.tests/GOLDEN_SYMBOL_TABLES")
set(GOLDEN_IR_DIR "${CMAKE_SOURCE_DIR}/.tests/GOLDEN_IR")
set(GOLDEN_COMPILE_ERRORS_DIR "${CMAKE_SOURCE_DIR}/.tests/GOLDEN_COMPILE_ERRORS")

function(make_dir_verbose dirname)
    file(MAKE_DIRECTORY ${dirname})
    if (EXISTS "${dirname}" AND IS_DIRECTORY "${dirname}")
        message(STATUS "Directory created successfully: ${dirname}")
    else ()
        message(FATAL_ERROR "Failed to create directory: ${dirname}")
    endif ()
endfunction()

make_dir_verbose("${AUTOGEN_ROOT_DIR_PATH}")

function(add_regression_check_target target_name do_memcheck)
    set(regression_args
        "--alpha-compiler" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ALPHA_DRIVER_EXECUTABLE}"
        "--golden-symbol-tables-dir" "${GOLDEN_SYMBOL_TABLES_DIR}"
        "--golden-ir-dir" "${GOLDEN_IR_DIR}"
        "--golden-compile-errors-dir" "${GOLDEN_COMPILE_ERRORS_DIR}"
        "--working-dir" "${WORKING_TESTS_DIR}"
        "--error-dir" "${ERROR_TESTS_DIR}"
    )
    if (do_memcheck)
        if (VALGRIND_PATH)
            list(APPEND regression_args "--memcheck")
        else ()
            message(WARNING "Memory check can not be performed as Valgrind not found!")
        endif ()
    endif ()

    add_custom_target(${target_name}
        COMMAND "${Python3_EXECUTABLE}"
        "-B"
        "${CMAKE_SOURCE_DIR}/scripts/alpha_regression_runner.py"
        ${regression_args}
    )

    add_dependencies(${target_name} ${ALPHA_DRIVER_EXECUTABLE})
endfunction()

if (NOT HATE_PYTHON_MODE)
    if (Python3_FOUND)
        add_regression_check_target(regression_check OFF)
        add_regression_check_target(regression_check_full ON)
    else ()
        message(WARNING "Regression check targets not available, as `Python3` was not found.")
    endif ()
else ()
    message(WARNING "Regression check targets not available in HATE_PYTHON_MODE.")
endif ()


# This property tells CMake to delete ${AUTOGEN_ROOT_DIR_PATH} and its contents when a "make clean" is run.
# set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES ${AUTOGEN_ROOT_DIR_PATH})

# Find all files stored in `${AUTOGEN_ROOT_DIR_PATH}` and delete them (but keep the directory structure intact).
file(GLOB_RECURSE AUTOGEN_FILES ${AUTOGEN_ROOT_DIR_PATH})
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${AUTOGEN_FILES}")

# === Core Compiler Components ===
add_subdirectory(core)
add_subdirectory(diagnostics)
add_subdirectory(parser)
add_subdirectory(scanner)
add_subdirectory(driver)

# === Utilities ===
add_subdirectory(arguinator)
add_subdirectory(utils)

# === External Dependencies ===
add_subdirectory(third_party)
