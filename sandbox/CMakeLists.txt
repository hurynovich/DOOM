project(sandbox VERSION 1.1.0)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer)

set(SOURCES
        src/sdl_audio_test.c
)

add_executable(sandbox ${SOURCES})

target_include_directories(sandbox PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(sandbox ${SDL2_LIBRARIES} m)
