cmake_minimum_required(VERSION 3.16)
project(DOOM)

set(CMAKE_C_STANDARD 99)

# Compiler flags from original Makefile with TLS and thread safety fixes
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNORMALUNIX -DLINUX -w -D_GNU_SOURCE -D_REENTRANT -w")

# Specifically disable optimization for Debug mode
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

# Ensure Release mode still uses optimization
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

# Add linker flags to handle TLS properly
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

# Include directories
include_directories(include)

add_subdirectory(linuxdoom)
add_subdirectory(sersrc)
add_subdirectory(sndserv)
add_subdirectory(ipx)
