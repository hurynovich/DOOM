### Recommended Reading Order

1.  `doomdef.h`: Start here to understand the basic types (`fixed_t`, `angle_t`).
2.  `d_main.c`: Read the `D_DoomLoop` to see how the engine "breathes."
3.  `r_main.c`: Look at `R_RenderPlayerView` for the big picture of rendering.
4.  `r_bsp.c`: Study `R_RenderBSPNode` to understand the 2.5D visibility magic.
5.  `r_draw.c`: Look at `R_DrawColumn` to see how pixels actually hit the buffer.

---

### 10 Beginner Questions to Ask Next

1.  How does `FixedMul` handle the bit-shifting for 16.16 math?
2.  What is the purpose of the `validcount` global variable used in many loops?
3.  How does the BSP tree allow for "front-to-back" rendering?
4.  What is a "seg" vs. a "linedef"?
5.  How are textures stored in memory (the `column_t` format)?
6.  How does `R_DrawSpan` calculate the texture coordinates for floors?
7.  What is the "Zone" memory allocator and why not just use `malloc`?
8.  How are sprites "projected" onto the screen?
9.  How does the engine handle multi-player networking (`ticcmd_t`)?
10. How does the status bar get updated without re-rendering the whole screen?

---

### 3 "Gotchas" for Modern C Programmers

1.  **Column-Major Textures**: DOOM stores textures as columns, not rows. This makes drawing vertical walls very fast but makes horizontal floors (spans) more complex.
2.  **Angle Overflow**: DOOM uses a 32-bit unsigned integer for angles, where `0xFFFFFFFF` is almost 360 degrees. It relies on integer wrap-around for modulo arithmetic.
3.  **The "Screen" is a Pointer**: `screens[0]` is just a big chunk of bytes. There are no "objects" or "buffers" in the modern OpenGL/Vulkan sense—just pointer arithmetic and raw pixel copying.
