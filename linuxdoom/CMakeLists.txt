project(linuxdoom VERSION 1.1.0)

# Explicitly list all source files from your Makefile
set(SOURCES
        src/doomdef.c
        src/doomstat.c
        src/dstrings.c
        src/i_system.c
        src/i_sound.c
        src/i_video.c
        src/i_net.c
        src/tables.c
        src/f_finale.c
        src/f_wipe.c
        src/d_main.c
        src/d_net.c
        src/d_items.c
        src/g_game.c
        src/m_menu.c
        src/m_misc.c
        src/m_argv.c
        src/m_bbox.c
        src/m_fixed.c
        src/m_swap.c
        src/m_cheat.c
        src/m_random.c
        src/am_map.c
        src/p_ceilng.c
        src/p_doors.c
        src/p_enemy.c
        src/p_floor.c
        src/p_inter.c
        src/p_lights.c
        src/p_map.c
        src/p_maputl.c
        src/p_plats.c
        src/p_pspr.c
        src/p_setup.c
        src/p_sight.c
        src/p_spec.c
        src/p_switch.c
        src/p_mobj.c
        src/p_telept.c
        src/p_tick.c
        src/p_saveg.c
        src/p_user.c
        src/r_bsp.c
        src/r_data.c
        src/r_draw.c
        src/r_main.c
        src/r_plane.c
        src/r_segs.c
        src/r_sky.c
        src/r_things.c
        src/w_wad.c
        src/wi_stuff.c
        src/v_video.c
        src/st_lib.c
        src/st_stuff.c
        src/hu_stuff.c
        src/hu_lib.c
        src/s_sound.c
        src/z_zone.c
        src/info.c
        src/sounds.c
        src/i_main.c
)

#Create executable
add_executable(linuxxdoom ${SOURCES})

# Link libraries (from your Makefile: -lXext -lX11 -lnsl -lm)
target_link_libraries(linuxxdoom Xext X11 nsl m)

# Compiler flags (from your Makefile)
target_compile_definitions(linuxxdoom PRIVATE NORMALUNIX LINUX)
target_compile_options(linuxxdoom PRIVATE -g -Wall)

# Linker flags (from your Makefile)
target_link_directories(linuxxdoom PRIVATE /usr/X11R6/lib)

# Set output directory to match original Makefile
set_target_properties(linuxxdoom PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/linux
)