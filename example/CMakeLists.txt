message(STATUS "Building oe example project.")

add_executable(example main.c)
target_link_libraries(example PUBLIC oe)

file(COPY assets DESTINATION .)
