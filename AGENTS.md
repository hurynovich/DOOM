# AGENTS.md — DOOM (Linux Port)

## Build

- **Pre-configured dir:** `cmake-build-debug/` (Ninja, Debug, created by CLion).
- **Build everything:** `cmake --build cmake-build-debug`
- **Build one target:** `cmake --build cmake-build-debug --target linuxxdoom`

## Targets

| Target       | Type       | Dir          | Notes                                      |
|--------------|------------|--------------|--------------------------------------------|
| `linuxxdoom` | executable | `linuxdoom/` | Main game (note the `x` — not `linuxdoom`) |
| `sndserv`    | executable | `sndserv/`   | Sound server                               |
| `sersrc`     | static lib | `sersrc/`    | Serial/modem net                           |
| `ipx`        | static lib | `ipx/`       | IPX net                                    |

## Source conventions

- **Includes are relative:** All source files use `#include "../include/foo.h"` — never rely on any global `-I` path.
- Root `CMakeLists.txt` has `include_directories(include)` but `include/` at root **does not exist**; that line is a
  no-op.
- Each subproject has its own `src/` and `include/` directory.

## Compiler/linker quirks

- `-w` suppresses **all** warnings. This is intentional — the codebase is 1990s C and produces many warnings under
  modern compilers.
- Defines: `NORMALUNIX`, `LINUX`, `_GNU_SOURCE`, `_REENTRANT`.
- Links X11: `-lXext -lX11 -lm` with `-L/usr/X11R6/lib`. Also needs `-pthread`.

## Runtime

- The game requires a WAD file. `freedoom1.wad` is provided in the repo root.
- Run: `./linuxdoom/linuxxdoom -iwad freedoom1.wad` (or copy WAD alongside the binary).

## Style

- `.clang-format` at root: C, LLVM base, indent 4, Mozilla braces, column limit 256.
- Files have historical CVS `$Id$` tags and Emacs mode lines — ignore them.

## No tests or CI

There is no test suite, no CI workflows, and no lint/format/typecheck commands.
