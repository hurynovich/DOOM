### C-Specific Patterns for Learning

*   **Global State**: DOOM is a "Global State Machine." Functions like `R_DrawColumn` take no parameters; they read `dc_x`, `dc_yl`, etc., from global variables. This was a performance optimization to avoid stack frame overhead.
*   **Fixed-Point Math**: Represented by `fixed_t` (a 32-bit integer). The lower 16 bits are the fractional part (`FRACBITS`). Multiplications use `FixedMul`. This was essential because 486/Pentium CPUs were much faster at integer math than floating point.
*   **Lookup Tables (LUTs)**: Sine/Cosine are precomputed in `finesine` (`tables.c`). Angle-to-pixel mappings are also precomputed. Trignometry was too expensive for real-time.
*   **Memory Assumptions**: Data from WAD files is often cast directly to C structs (e.g., `mapnode_t`). This assumes specific byte ordering and alignment, which is a "gotcha" for modern cross-platform development.

---

### Historical vs. Portability

*   **Core Engine**: `r_*.c`, `p_*.c`, `d_*.c`. This is the mathematical "heart" of DOOM. It doesn't know about Linux or Windows.
*   **Portability Layer**: `i_video.c` (X11), `i_sound.c` (Linux audio). These are the "muscles" that talk to the hardware.
*   **Historical Baggage**: You may see references to `DMX` (the original sound library) or `DOS` specific code. In this Linux version, these are either replaced or stubbed.
