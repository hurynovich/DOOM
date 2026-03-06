### The Renderer Pipeline: Step-by-Step

#### Step 1: View Setup
*   **Function**: `R_SetupFrame` (`r_main.c`)
*   **Problem**: Prepares the camera and math tables for the current frame.
*   **Inputs**: `player_t* player`.
*   **Global State**: Sets `viewx`, `viewy`, `viewz`, `viewangle`.
*   **Next**: `R_RenderBSPNode`.

#### Step 2: BSP Traversal (Visibility)
*   **Function**: `R_RenderBSPNode` (`r_bsp.c`)
*   **Problem**: Determines which parts of the map are visible in front-to-back order (occlusion culling).
*   **Inputs**: Root node index.
*   **Global State**: Reads `viewx`, `viewy`.
*   **Next**: Calls itself recursively or `R_Subsector` when a leaf is reached.

#### Step 3: Wall Segment Clipping
*   **Function**: `R_AddLine` (`r_bsp.c`) / `R_StoreWallRange` (`r_segs.c`)
*   **Problem**: Takes a wall segment, clips it to the 2D view cone, and calculates its screen projection.
*   **Inputs**: `seg_t* line`.
*   **Global State**: Reads `viewangle`, `viewx`, `viewy`. Writes to `drawsegs`.
*   **Next**: `R_DrawColumn` (low-level).

#### Step 4: Floor/Ceiling Drawing
*   **Function**: `R_DrawPlanes` (`r_plane.c`)
*   **Problem**: Fills horizontal surfaces (planes).
*   **Inputs**: List of visible subsectors gathered during BSP traversal.
*   **Global State**: Uses `visplanes` array.
*   **Next**: Calls `R_DrawSpan` (`r_draw.c`).

#### Step 5: Sprite Drawing
*   **Function**: `R_DrawMasked` (`r_things.c`)
*   **Problem**: Draws all "things" (monsters, items) and transparent walls, sorted back-to-front.
*   **Inputs**: List of `vissprites`.
*   **Global State**: Uses `vissprites` list.
*   **Next**: Final pixels are in `screens[0]`.
