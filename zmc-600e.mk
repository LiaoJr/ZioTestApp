MESSAGE(STATUS "BUILD FOR ZMC600E")
#across compiler setting
#include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER    aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER  aarch64-linux-gnu-g++)
set(CMAKE_LINKER        aarch64-linux-gnu-ld)
set(CMAKE_AR            aarch64-linux-gnu-ar)
set(CMAKE_RANLIB        aarch64-linux-gnu-ranlib)

#set(CMAKE_FIND_ROOT_PATH ${TOOL_CHAIN_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
