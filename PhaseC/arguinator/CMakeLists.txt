add_library(arguinator STATIC
    src/arguinator.cpp
)

target_include_directories(arguinator PRIVATE
    "include"
)

target_link_libraries(arguinator PRIVATE
    utils
)
