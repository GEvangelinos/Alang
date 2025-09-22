get_filename_component(CURRENT_COMPONENT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
set(GENERATED_INCLUDE_DIR "${AUTOGEN_ROOT_DIR_PATH}/${CURRENT_COMPONENT_NAME}/include")

# === alpha_settings_api: Interface (public headers only) ===
add_library(alpha_settings_api INTERFACE)
target_include_directories(alpha_settings_api INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)


# === alpha_settings_impl: Implementation (compiled source) ===
add_library(alpha_settings_impl STATIC
    src/compiler_settings.cpp
)
target_link_libraries(alpha_settings_impl PRIVATE
    alpha_settings_api
    arguinator_api
    alpha_driver_api
    support_api
)
