# === alpha_driver_api: Interface (public headers only) ===
add_library(alpha_vm_api INTERFACE)
target_include_directories(alpha_vm_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(alpha_vm_api INTERFACE
    alpha_core_api
)

# === alpha_driver_impl: Implementation (compiled source) ===
add_library(alpha_vm_impl STATIC
    src/machine.cpp
)
target_link_libraries(alpha_vm_impl PRIVATE
    alpha_vm_api
)

# === Executable ===
add_executable("${VM_EXE_NAME}"
    src/main.cpp
)
target_link_libraries("${VM_EXE_NAME}" PRIVATE
)

target_compile_options(${VM_EXE_NAME} PRIVATE -flto)
target_link_options(${VM_EXE_NAME} PRIVATE -flto)
