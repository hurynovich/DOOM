project(sndserv VERSION 1.0.0)

set(SOURCES
        src/linux.c
        src/sounds.c
        src/soundsrv.c
        src/wadread.c
)

add_executable(sndserv ${SOURCES})

target_include_directories(sndserv PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(sndserv ${SDL2_LIBRARIES} m)