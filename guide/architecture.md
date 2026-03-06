### 1. Project Map: Subsystems & Responsibilities

The DOOM engine is divided into logical modules, mostly identified by their file prefixes:

*   **D (Main/Doom)**: `d_main.c`, `d_net.c`. High-level game coordination, command-line parsing, and the main game loop.
*   **R (Renderer)**: `r_main.c`, `r_bsp.c`, `r_segs.c`, `r_plane.c`, `r_things.c`. The "Refresh" system. Responsible for 3D visibility (BSP) and drawing walls, floors, and sprites.
*   **P (Playsim)**: `p_tick.c`, `p_user.c`, `p_map.c`. The game world simulation: physics, collision detection, and monster AI.
*   **I (System Interface)**: `i_video.c`, `i_sound.c`, `i_system.c`. The portability layer. Handles OS-specific tasks like blitting pixels to the screen or getting keyboard input.
*   **W (WAD)**: `w_wad.c`. Resource management. Handles loading data (lumps) from the `.wad` files.
*   **V (Video)**: `v_video.c`. Low-level surface management and drawing primitives (patches/rectangles).
*   **Z (Zone)**: `z_zone.c`. A custom memory allocator designed to handle the frequent loading/unloading of game assets.

---

### 2. Runtime Flow: From Start to Frame

1.  **Entry Point**: `main` in `i_main.c` immediately calls `D_DoomMain`.
2.  **Initialization**: `D_DoomMain` (`d_main.c`) initializes subsystems in order: Video -> Memory -> WADs -> Renderer -> Playsim -> Sound -> HUD.
3.  **The Loop**: `D_DoomLoop` (`d_main.c`) starts a `while(1)` loop:
    *   `I_StartFrame`: Polls system events (keyboard/mouse).
    *   `TryRunTics`: Updates the game logic (Playsim) at a fixed 35 FPS rate.
    *   `D_Display`: The entry to the rendering process.
    *   `I_FinishUpdate`: Blits the final buffer to the window.
