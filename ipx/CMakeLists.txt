project(ipx VERSION 1.1.0)

set(SOURCES
        src/doomnet.c
        src/ipxnet.c
        src/ipxsetup.c
)

add_library(ipx ${SOURCES})