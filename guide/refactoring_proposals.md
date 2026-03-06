# Strategic Refactoring Proposals for the DOOM Engine

This document outlines high-level architectural and structural changes to modernize the 1994 DOOM source code, taking advantage of modern C standards (C99/C11/C17) and current hardware capabilities.

---

### 1. Adopt Standard Fixed-Width Integers (`stdint.h`)

*   **Proposed Change**: Replace all platform-dependent types like `int`, `short`, and custom `byte` with standard C99 types: `int32_t`, `int16_t`, and `uint8_t`.
*   **Original Implementation**: In 1994, the C89 standard was prevalent, and `stdint.h` did not exist. Programmers relied on the fact that `int` was 32-bit and `short` was 16-bit on the 486/Pentium target platforms.
*   **Why It's Better**: Modern platforms (64-bit, ARM, RISC-V) have different default bit-widths and alignment rules. Using `stdint.h` guarantees that binary data loaded from WAD files (like map nodes or textures) matches the memory layout of the structs, preventing subtle bugs and crashes on non-x86 architectures.

---

### 2. Encapsulation of Global State

*   **Proposed Change**: Group related global variables (e.g., `viewx`, `viewy`, `viewangle`, `visplanes`) into context structures (e.g., `render_context_t`, `game_state_t`) and pass pointers to these structures to functions.
*   **Original Implementation**: DOOM was designed as a "Global State Machine" to maximize performance. Passing pointers to structures on 486 CPUs incurred significant overhead from stack frame management and increased register pressure.
*   **Why It's Better**: Modern CPUs have large L1/L2 caches and sophisticated branch predictors that make the overhead of passing a pointer negligible. Encapsulation enables multi-threading (e.g., rendering multiple views simultaneously for split-screen), simplifies testing/debugging, and makes the data flow explicitly clear to the developer.

---

### 3. Transition from Zone Allocator to Modern Memory Management

*   **Proposed Change**: Replace the custom `z_zone.c` memory manager with modern `malloc`/`free` or an arena-based allocator from a library like `jemalloc` or `mimalloc`.
*   **Original Implementation**: Early operating systems (MS-DOS) had primitive memory management. The "Zone" allocator was a custom solution to manage large, frequently loaded/unloaded WAD assets in very limited RAM (4-8MB) without fragmentation, using manual "purgeable" and "cache" tags.
*   **Why It's Better**: Modern OS allocators are extremely efficient and thread-safe. The manual tagging in `z_zone` is a common source of memory leaks and double-free errors. Standard tools like Valgrind or AddressSanitizer (ASan) work better with standard allocators, making it easier to maintain a "leak-free" engine.

---

### 4. Replace Macros with `static inline` and `enum`

*   **Proposed Change**: Convert complex `#define` math macros (like `FixedMul`) to `static inline` functions and replace constant `#define`s with `enum` types.
*   **Original Implementation**: In 1994, `inline` was not a part of the C standard (it was C++ only). Macros were the only way to avoid the cost of a function call in performance-critical loops.
*   **Why It's Better**: Macros lack type safety and are difficult to debug because they disappear during preprocessing. `static inline` functions provide the same performance benefits while allowing the compiler to perform type checking and generating debug symbols that modern IDEs can use for navigation and inspection.

---

### 5. Proper Serialization for WAD Data (Endians & Alignment)

*   **Proposed Change**: Implement a serialization layer that reads bytes from WAD files and populates C structs explicitly, rather than casting raw memory pointers directly to struct types.
*   **Original Implementation**: For speed, DOOM would read a lump from the WAD file into memory and cast the pointer directly to a struct (e.g., `(mapnode_t*)data`). This assumed the WAD was little-endian and the compiler would not add padding between struct members.
*   **Why It's Better**: This pattern is "undefined behavior" in modern C and is highly non-portable. It fails on big-endian systems (PowerPC) and architectures that require strict memory alignment (ARM). Explicit serialization makes the data loading process robust and cross-platform.

---

### 6. Switch to Floating-Point Math for Rendering

*   **Proposed Change**: Replace the `fixed_t` (16.16 fixed-point) math system in the renderer with `float` or `double`.
*   **Original Implementation**: Most 486 CPUs in the early 90s lacked a dedicated Floating Point Unit (FPU). Integer math was significantly faster for real-time graphics.
*   **Why It's Better**: Modern CPUs have powerful FPUs and SIMD units (SSE/AVX/NEON) that handle floating-point math at the same speed as integers. Using floats eliminates "fixed-point overflow" bugs, improves precision (reducing "jittery" walls), and makes it trivial to integrate the engine with modern graphics APIs like OpenGL or Vulkan.

---

### 7. Modernize the Platform Layer (SDL2)

*   **Proposed Change**: Replace the direct X11 (`i_video.c`) and OSS sound (`i_sound.c`) code with a modern abstraction library like SDL2 (Simple DirectMedia Layer).
*   **Original Implementation**: When the Linux port was released, X11 and OSS were the standard ways to interact with hardware on Linux.
*   **Why It's Better**: SDL2 provides a unified, cross-platform API for video, audio, and input. It handles modern requirements like window resizing, high-DPI displays, multiple monitors, and modern game controllers (gamepads) that the original engine's I/O layer simply cannot support without a massive rewrite.
