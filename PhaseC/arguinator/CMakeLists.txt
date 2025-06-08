add_library(arguinator STATIC
    src/arguinator.cpp
)

target_include_directories(arguinator PUBLIC
    "include"
)

target_link_libraries(arguinator PUBLIC
    utils
)
