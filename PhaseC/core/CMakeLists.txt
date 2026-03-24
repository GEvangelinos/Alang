add_library(alpha_core_api INTERFACE)
target_include_directories(alpha_core_api INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(alpha_core_api INTERFACE support_api alpha_parser_api)

add_library(alpha_core_impl STATIC
    src/source_location.cpp
    src/source_location_tracker.cpp
    src/translation_unit_buffer.cpp
    src/ir/ir_expr.cpp
    src/bytecode/vm_opcodes.cpp
)
target_link_libraries(alpha_core_impl PRIVATE alpha_core_api)
