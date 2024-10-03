file(COPY shaders DESTINATION .)

# ~ build dependencies
find_package(Vulkan REQUIRED FATAL_ERROR)
add_subdirectory(deps)

# ~ print info
message(STATUS "==== OE info ====")
message(STATUS "version: ${OE_VERSION}")

# ~ setup library
set(OE_HEADER_FILES
  ./include/oe.h
  ./src/internal.h
)

set(OE_SOURCE_FILES
  ./src/log.c
  ./src/gfx.c
  ./src/init.c
  ./src/utils.c
  ./src/input.c
)

if (OE_SHARED)
  set(OE_LIB_TYPE SHARED)
  message(STATUS "lib type: shared")
else()
  set(OE_LIB_TYPE STATIC)
  message(STATUS "lib type: static")
endif()

add_library(oe ${OE_LIB_TYPE} ${OE_SOURCE_FILES} ${OE_HEADER_FILES})

target_link_libraries(oe PRIVATE opl archivio stb_image Vulkan::Vulkan)
target_include_directories(oe PUBLIC include)
target_compile_options(
  oe PRIVATE
  -Wall -Wextra -Wpedantic -Werror -Wno-gnu-zero-variadic-macro-arguments
)

# ~ add compile definitions
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(
    STATUS
    "Building oe for debug. Runtime asserts, trace and debug "
    "messages are enabled."
  )
  target_compile_definitions(oe PUBLIC OE_DEBUG_BUILD)
else()
  message(STATUS "Building oe for release.")
  target_compile_definitions(oe PUBLIC OE_RELEASE_BUILD)
endif()

if (APPLE)
  target_compile_definitions(oe PRIVATE OE_PLATFORM_MACOS)
endif()

