cmake_minimum_required(VERSION 3.16)

# ~ setup cmake options
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ~ setup project
set(OE_VERSION_MAJOR 0)
set(OE_VERSION_MINOR 1)
set(OE_VERSION_PATCH 0)
set(
  OE_VERSION
  "${OE_VERSION_MAJOR}.${OE_VERSION_MINOR}.${OE_VERSION_PATCH}"
)

project(oe VERSION ${OE_VERSION} LANGUAGES C)

# ~ include options
include(CMakeOptions.txt)

# ~ add subdirs
add_subdirectory(runtime)

if (OE_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

