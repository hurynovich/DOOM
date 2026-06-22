# DOOM Sound Subsystem Architecture

## Overview

The sound system has a **3-layer architecture** with two distinct operating modes:

| Layer                       | File                      | Role                                                              |
|-----------------------------|---------------------------|-------------------------------------------------------------------|
| **Game logic**              | `linuxdoom/src/s_sound.c` | Decides *what* to play (distance, priority, channels)             |
| **Hardware abstraction**    | `linuxdoom/src/i_sound.c` | Mixes sounds into PCM buffer, talks to `/dev/dsp` or sound server |
| **Sound server** (optional) | `sndserv/src/soundsrv.c`  | Separate process that does mixing + output via pipe protocol      |

### Related Headers

| Header       | Location                      |
|--------------|-------------------------------|
| `s_sound.h`  | `linuxdoom/include/s_sound.h` |
| `i_sound.h`  | `linuxdoom/include/i_sound.h` |
| `sounds.h`   | `linuxdoom/include/sounds.h`  |
| `soundsrv.h` | `sndserv/include/soundsrv.h`  |
| `soundst.h`  | `sndserv/include/soundst.h`   |

---

## Two Operating Modes

### 1. SNDSERV Mode (Separate Process)

The main game spawns `sndserv` via `popen()`. Communication is a text protocol over stdin:

- `p<sfxid><pitch><vol><sep>\n` — play a sound (hex-encoded, 8 hex chars)
- `q\n` — quit
- `s<sfxid>\n` — save raw SFX data to file (debug)

The server reads its own WAD, loads all SFX at startup, and runs a continuous mixing loop calling `mix()` ->
`I_SubmitOutputBuffer()` -> `write()` to `/dev/dsp`.

The sound server is selected at compile time via the `SNDSERV` preprocessor define.

### 2. Direct Mode (No SNDSERV)

`i_sound.c` opens `/dev/dsp` directly, pre-caches all SFX at startup via `getsfx()`, and mixes in `I_UpdateSound()`
using 8 internal channels into a 512-sample stereo 16-bit `mixbuffer`. `I_SubmitSound()` writes it to the DSP device.

There is also an experimental `SNDINTR` mode that uses a POSIX timer (`SIGALRM` / `ITIMER_REAL`) to asynchronously
submit sound buffers at ~500 microsecond intervals.

---

## Data Flow (SFX)

```
Game event (e.g. monster attack, door open)
  |
  v
S_StartSound(origin, sfx_id)
  |
  v
S_StartSoundAtVolume(origin, sfx_id, volume)
  |
  +-- S_AdjustSoundParams() -- calculates distance attenuation and stereo separation
  |     - Euclidean distance approximation: adx + ady - min(adx,ady)/2
  |     - Clipping distance: 1200 units (sounds beyond this are silent)
  |     - Close distance: 160 units (sounds within this are max volume)
  |     - Stereo: angle-based separation using finesine table
  |
  +-- S_getChannel() -- finds an available channel or evicts lowest priority
  |
  v
I_StartSound(id, vol, sep, pitch, priority)
  |
  +-- SNDSERV mode: fprintf(sndserver, "p%02x%02x%02x%02x\n", id, pitch, vol, sep)
  |
  +-- Direct mode: addsfx() -- sets channel pointers into raw WAD data
```

---

## Mixing Algorithm

Both `i_sound.c` and `soundsrv.c` use the same core algorithm:

1. **8 hardware channels**, each pointing to raw 8-bit unsigned PCM data from the WAD
2. **Volume lookup table** (`vol_lookup[128*256]`): converts unsigned samples to signed and applies volume in one step.
   Formula: `(volume * (sample - 128) * 256) / 127`
3. **Stereo panning**: x^2 separation law. Left and right volumes are computed as:
    - `leftvol = volume - (volume * sep^2) / (256^2)`
    - `rightvol = volume - (volume * (sep-257)^2) / (256^2)`
4. **Pitch stepping**: `channelstepremainder` accumulates `channelstep` each sample; the MSB (>>16) advances the data
   pointer. This allows pitch shifting.
5. **512 samples per mix pass**, interleaved L/R 16-bit signed output
6. **Sample rate**: 11025 Hz
7. **Clamping**: output clamped to [-0x8000, 0x7fff]

### Mix Buffer Layout

```
MIXBUFFERSIZE = SAMPLECOUNT * 4 = 512 * 4 = 2048 bytes
              = 512 samples * 2 channels * 2 bytes (16-bit)

mixbuffer: [L0][R0][L1][R1]...[L511][R511]
```

---

## Sound Data Loading

At startup, all SFX are loaded from the WAD:

1. Each sound effect has a name like `pistol`, `shotgn`, etc. (defined in `sounds.h`)
2. The WAD lump name is `ds` + name (e.g., `dspistol`, `dsshotgn`)
3. Raw data is 8-bit unsigned PCM with an 8-byte header
4. Data is padded to a multiple of `SAMPLECOUNT` (512) with silence (value 128)
5. Linked sounds (aliases) share data with their parent (e.g., chaingun links to pistol)
6. Sounds not found in the WAD fall back to `dspistol`

---

## Channel Management

### Logical Channels (s_sound.c)

- `numChannels` channels allocated in `S_Init()` (value set externally)
- Each channel tracks: `sfxinfo` pointer, `origin` (mobj_t*), `handle`
- Priority-based eviction: if all channels busy, lowest priority sound is replaced
- Same-origin sounds stop the previous sound before playing new one
- "Singularity" sounds (chainsaw, pistol, etc.) only play once at a time

### Hardware Channels (i_sound.c / soundsrv.c)

- Fixed 8 channels
- Oldest sound is evicted when all channels are busy
- Special handling for chainsaw/pistol/stepping sounds to avoid duplicates
- Each channel has: data pointer, end pointer, step, step remainder, start time, handle, left/right volume lookups, SFX
  id

---

## Music Subsystem

Music is **stubbed out** in the Linux port:

- `I_RegisterSong()`, `I_PlaySong()`, `I_StopSong()`, etc. are all dummy functions
- They track a `musicdies` gametic timer (30 seconds) but produce no actual audio
- `S_ChangeMusic()` in `s_sound.c` loads MUS lumps from the WAD (prefixed `d_`, e.g., `d_e1m1`) and calls the I_* music
  API, but nothing plays
- Music selection is based on game episode/map:
    - DOOM 1: `mus_e1m1 + (episode-1)*9 + (map-1)`
    - DOOM 2: `mus_runnin + map - 1`
    - Episode 4 uses a special mapping table

---

## Key Constants

| Constant          | Value      | Description              |
|-------------------|------------|--------------------------|
| `SAMPLECOUNT`     | 512        | Samples per mix pass     |
| `MIXBUFFERSIZE`   | 2048       | Mix buffer size in bytes |
| `SAMPLERATE`      | 11025 Hz   | Audio output rate        |
| `SAMPLESIZE`      | 2 bytes    | 16-bit samples           |
| `NUM_CHANNELS`    | 8          | Hardware mixing channels |
| `S_CLIPPING_DIST` | 1200 units | Max audible distance     |
| `S_CLOSE_DIST`    | 160 units  | Distance for max volume  |
| `S_STEREO_SWING`  | 96 units   | Stereo separation range  |
| `S_MAX_VOLUME`    | 127        | Maximum volume level     |
| `NORM_PITCH`      | 128        | Normal pitch (no shift)  |
| `NORM_SEP`        | 128        | Center stereo separation |

---

## Sound Effect Catalog

All 136 sound effects are enumerated in `sounds.h` as `sfxenum_t`:

- Weapons: `sfx_pistol`, `sfx_shotgn`, `sfx_plasma`, `sfx_bfg`, `sfx_saw*`, `sfx_rlaunc`, etc.
- Doors: `sfx_doropn`, `sfx_dorcls`, `sfx_stnmov`
- Player: `sfx_plpain`, `sfx_pldeth`, `sfx_pdiehi`, `sfx_oof`, `sfx_noway`
- Monsters: sit/attack/death sounds for each monster type
- Environment: `sfx_telept`, `sfx_itemup`, `sfx_wpnup`, `sfx_barexp`, etc.

---

## Shutdown

- **SNDSERV**: sends `q\n` command to sound server process via pipe
- **Direct**: closes `/dev/dsp` file descriptor (pending sounds are NOT finished — noted as FIXME in code)
- **SNDINTR**: additionally removes the timer via `I_SoundDelTimer()`
