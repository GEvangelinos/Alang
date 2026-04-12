# === alpha_bytecode_generator_api: Interface (public headers only) ===
add_library(alpha_bytecode_generator_api INTERFACE)
target_include_directories(alpha_bytecode_generator_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(alpha_bytecode_generator_api INTERFACE
    alpha_core_api
)

# === alpha_bytecode_generator_impl: Implementation (compiled source) ===
add_library(alpha_bytecode_generator_impl STATIC
    src/bytecode_generator.cpp
    src/abc_serializer.cpp
)
target_link_libraries(alpha_bytecode_generator_impl PRIVATE
    alpha_bytecode_generator_api
)
