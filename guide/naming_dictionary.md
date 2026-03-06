### DOOM Naming Dictionary & Lexicon

To understand the DOOM codebase, one must learn its specific vocabulary. This dictionary breaks down the prefixes, abbreviations, and terminology used throughout the engine.

---

### 1. Subsystem Prefixes (The "Namespaces")

Most global functions and many variables start with a 1-2 letter prefix identifying their module:

| Prefix | Subsystem | Description |
| :--- | :--- | :--- |
| **D_** | **Doom** | Main entry point, high-level game loop (`d_main.c`). |
| **R_** | **Refresh** | The Renderer (`r_main.c`, `r_bsp.c`, etc.). |
| **P_** | **Play** | The game logic simulation/playsim (`p_tick.c`, `p_map.c`). |
| **W_** | **WAD** | Resource management and file loading (`w_wad.c`). |
| **I_** | **Interface** | OS-specific code (Video/Sound/System) (`i_video.c`). |
| **V_** | **Video** | Low-level screen buffering and blitting (`v_video.c`). |
| **Z_** | **Zone** | Memory management system (`z_zone.c`). |
| **M_** | **Misc** | Common utility functions, menu handling (`m_misc.c`). |
| **G_** | **Game** | Game-level operations like saving/loading/demos (`g_game.c`). |
| **S_** | **Sound** | High-level sound effects and music handling (`s_sound.c`). |
| **HU_** | **Heads-Up** | The Heads-Up Display (messages and chat). |
| **ST_** | **Status** | The status bar at the bottom of the screen. |
| **AM_** | **AutoMap** | The interactive map overlay. |
| **WI_** | **Intermission** | The "stats" screen between levels. |
| **F_** | **Finale** | The end-game text and animations. |

---

### 2. Common Variable & State Prefixes

These are often used as abbreviations in global variables and struct members:

*   **dc_** (Draw Column): Global parameters passed to vertical wall/sprite drawing functions (e.g., `dc_yl`, `dc_yh`).
*   **ds_** (Draw Seg): Related to the `drawseg_t` structure, which stores a visible wall segment.
*   **rw_** (Render Wall): Intermediate calculations for walls (e.g., `rw_distance`, `rw_scale`).
*   **vis** (Visible): Objects currently marked for rendering (e.g., `visplanes`, `vissprites`).
*   **num** (Number): Always used as a count (e.g., `numnodes`, `numsubsectors`).
*   **gametic**: The internal clock of the game logic.

---

### 3. Core Nouns (The Domain Model)

*   **WAD**: "Where's All the Data?" The main archive format for levels, graphics, and sound.
*   **Lump**: A single entry/file within a WAD archive.
*   **Mobj** (Map Object): The internal name for all actors (monsters, players, fireballs, items).
*   **Tic**: A single unit of game time (1/35th of a second).
*   **Linedef**: A line on the 2D map that defines a boundary.
*   **Sidedef**: The data associated with one side of a Linedef (texture name, offsets).
*   **Sector**: A 2D area with specific ceiling/floor heights and light levels.
*   **Subsector**: A smaller convex area created by the BSP process.
*   **Seg** (Segment): A part of a Linedef that forms a wall of a Subsector.
*   **Patch**: A standard 2D image format used for menus and sprites.
*   **Flat**: A raw image format specifically used for horizontal floors and ceilings.
*   **Node**: A split-point in the BSP tree.

---

### 4. Common Verbs (Action Functions)

*   **Init**: One-time setup for a system.
*   **Ticker**: The logic update function called 35 times per second.
*   **Drawer**: The visual update function (blits stuff to buffers).
*   **Setup**: Prepares state for a specific task (e.g., `R_SetupFrame`).
*   **Add**: Pushes a new item into a list or buffer (`R_AddSprites`).
*   **Check**: Performs a conditional test (`M_CheckParm`, `R_CheckBBox`).
*   **Store**: Finalizes calculations and saves them (`R_StoreWallRange`).

---

### 5. Cryptic Abbreviations Decoder

| Abbreviation | Meaning | Use Case |
| :--- | :--- | :--- |
| **yl / yh** | Y Low / Y High | Vertical screen coordinates (bottom/top). |
| **x1 / x2** | X coordinate range | Horizontal screen boundaries for a wall. |
| **mo** | Map Object | Pointer to an actor (`mobj_t*`). |
| **ss** | Subsector | Index or pointer for BSP leaf areas. |
| **v1 / v2** | Vertex 1 / Vertex 2 | Endpoints of a line or segment. |
| **fixed** | Fixed-Point | 16.16 integer math representation. |
| **frac** | Fraction | The decimal part of a fixed-point number. |
| **bbox** | Bounding Box | Array of 4 values (top, bottom, left, right). |
| **PU_** | Purge Unit | Memory tags for the Zone allocator (e.g., `PU_CACHE`). |

---

### Analyst's Note
One of the most surprising findings is how often the code relies on **one-letter variables** (like `p` for player, `s` for sector) within long functions. This was a common optimization to keep registers efficient on 486 compilers, but it makes the code hard to read without context. Always look at the function's prefix to understand what `p` likely points to!