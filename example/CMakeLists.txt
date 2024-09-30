message(STATUS "Building oe example project.")

add_executable(example main.c)
target_link_libraries(example PRIVATE oe)
