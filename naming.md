# Naming Conventions

## Source filed prefixes

| Prefix | Expanded     | Description                                  |
|--------|--------------|----------------------------------------------|
| d_     | Doom         | High-level game entry and global definitions |
| i_     | Interface    | Platform-specific OS/IO abstraction layer    |
| z_     | Zone         | Custom memory allocator and heap system      |
| w_     | WAD          | Resource loader for WAD file archives        |
| r_     | Render       | Software renderer (walls, sprites, planes)   |
| p_     | Play         | Core gameplay logic and world simulation     |
| m_     | Misc/Menu    | Menus, config, utilities, and misc helpers   |
| s_     | Sound        | High-level sound control system              |
| g_     | Game         | Game session control and state management    |
| v_     | Video        | Low-level screen/framebuffer operations      |
| hu_    | HUD          | Heads-up display rendering and logic         |
| st_    | Status       | Status bar rendering and updates             |
| am_    | Automap      | Automap rendering and interaction            |
| wi_    | Intermission | Post-level statistics and screens            |
| f_     | Finale       | Ending sequences and ending cutscenes        |
| in_    | Intermission | Level transition and episode intermissions   |