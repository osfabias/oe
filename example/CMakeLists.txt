message(STATUS "Building oe example project.")

add_executable(
  example
  main.c
  ./src/core/room.c
  ./src/core/tilemap.c

  ./src/game/player.c
)
target_link_libraries(example PUBLIC oe)

target_compile_options(
  example PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-gnu-zero-variadic-macro-arguments
)

include_directories(src)

file(COPY assets DESTINATION .)
