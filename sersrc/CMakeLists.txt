project(sersrc VERSION 1.0.0)

set(SOURCES
        src/doomnet.c
        src/port.c
        src/sersetup.c
)

add_library(sersrc ${SOURCES})