# Osfabias Engine
OE is an Open-Source, multi-platform C library for developing
high-performance cross-platform 2D graphics applications.

Currently supported platforms:
- MacOS.

## Features
- Multithreaded logging
- Optimized 2D rendering

# Quick start
Requirements:
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (latest version)
- CMake (version 2.16+)
- GLSLC

### 1. Clone project and submodules.
```shell
git clone git@github.com:osfabias/oe.git
cd oe
git submodule init
git submodule update
```

### 2. Configure and build project via CMake.
```shell
# from oe/
mkdir build
cd build
cmake .. # add -DCMAKE_BUILD_TYPE=Debug for debug build
cmake --build . -j 8
```

### 3. Start binary.
```shell
# from oe/build/
cd bin
./example
```

You're done!

# Testing
For testing OE you need to firstly build project.
Than use this commands to start ctest.

```shell
# from oe/
cd build/tests
ctest
```

# Documentation
```shell
# from oe/
doxygen
cd doc/html
open index.html
```

