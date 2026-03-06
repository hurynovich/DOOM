### Porting DOOM to STM32: Difficulty and Plan

Refactoring the DOOM engine for a microcontroller like the STM32 is a classic "rite of passage" for embedded developers. While the core engine is written in portable C, the primary challenge is not the code itself, but the hardware constraints of an embedded environment.

---

### 1. Difficulty Estimation

**Difficulty: High / Advanced**

The task is difficult primarily due to **RAM constraints**.
*   **Memory**: The original DOOM requires ~4-8MB of RAM. Most STM32 chips have between 128KB and 2MB of internal RAM. You will likely need an STM32 with an external memory interface (FMC/FSMC) and an external SDRAM chip, or you will need to heavily rewrite the resource management logic to "stream" data from an SD card.
*   **Storage**: The full game data (WAD file) is ~12MB (Shareware) to ~14MB (Registered). This won't fit in internal Flash, so an SD card and a filesystem (like FatFs) are mandatory.
*   **Performance**: While a 200MHz+ STM32 (Cortex-M7) is faster than a 1994 486 PC, the lack of a GPU means you are doing all pixel pushing via software, which tests the limits of the SPI/Parallel bus to your DIY display.

---

### 2. Refactoring Plan

#### Step 1: Isolate the Platform Layer (The "I" Subsystem)
*   **What**: Identify all functions in `i_video.c`, `i_system.c`, and `i_sound.c`. Create a new platform file (e.g., `i_stm32.c`).
*   **Why**: These files contain all the Linux-specific code (X11, POSIX sockets, `gettimeofday`). You need to replace these with calls to the STM32 HAL or CMSIS libraries.

#### Step 2: Implement a Custom Memory Strategy
*   **What**: Modify `I_ZoneBase` in `i_system.c`. If you have external SDRAM, point the Zone allocator there. If not, you must modify `z_zone.c` to use a much smaller heap and implement aggressive "lump" (asset) unloading.
*   **Why**: The engine expects a large, contiguous block of memory. On a microcontroller, you must manually manage where the heap lives.

#### Step 3: Abstract the Filesystem (WAD Loading)
*   **What**: Replace standard C file I/O (`fopen`, `fread`, `fseek`) in `w_wad.c` with an abstraction layer that uses a library like **FatFs**.
*   **Why**: Microcontrollers don't have a native OS filesystem. You need to read the WAD data from an SD card via SPI or SDIO.

#### Step 4: Develop the Display Driver
*   **What**: Re-implement `I_FinishUpdate`. Instead of blitting to an X11 window, you will transmit the buffer (`screens[0]`) to your DIY display over SPI, 8-bit parallel, or LTDC (if supported).
*   **Why**: This is the most performance-critical part. You may need to use **DMA (Direct Memory Access)** to send pixels to the display while the CPU starts calculating the next frame.

#### Step 5: Map the Keypad Input
*   **What**: In `I_GetEvent`, poll the GPIO pins connected to your keypad. Convert button presses into DOOM events (`ev_keydown`, `ev_keyup`) and push them into the engine's event queue.
*   **Why**: DOOM has a built-in event queue system. By feeding it hardware interrupts or polled GPIO state, the rest of the engine (menus, movement) will work without modification.

#### Step 6: Hardware Timer Synchronization
*   **What**: Replace `I_GetTime` with a call to an STM32 hardware timer or the SysTick counter, calibrated to the engine's 35Hz tic rate.
*   **Why**: DOOM's game logic (`P_Ticker`) is strictly tied to a fixed 35 FPS clock. Accurate timing is essential for the game to run at the correct speed.

#### Step 7: Floating-Point vs. Fixed-Point Check
*   **What**: Decide whether to keep the `fixed_t` (16.16) math or move to `float`.
*   **Why**: If your STM32 has a Hardware FPU (Cortex-M4F/M7), floating-point math might actually be faster than the bit-shifting required for fixed-point math. However, keeping fixed-point is "safer" for initial porting as it avoids changing the core engine logic.

---

### Summary Recommendation for STM32
Start by getting the **Shareware WAD** loading first. Use a serial port (UART) for `printf` output to debug the initialization sequence before you even touch the DIY display. Once you see "Z_Init: Init zone memory" in your terminal, you know the heart of the engine is beating!