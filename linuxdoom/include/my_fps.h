#ifndef FPS_H
#define FPS_H

#include <stddef.h>

/**
 * Opaque type representing FPS tracking state.
 */
typedef struct fps_state fps_state_t;

/**
 * Create a new FPS tracker.
 *
 * Allocates and initializes FPS state. The caller is responsible for
 * calling fps_destroy() when done.
 *
 * Returns: Pointer to new fps_state_t, or NULL on allocation failure.
 */
extern fps_state_t *fps_create(void);

/**
 * Destroy an FPS tracker and free associated memory.
 *
 * Parameters:
 *   state - Pointer to fps_state_t to destroy. May be NULL.
 */
void fps_destroy(fps_state_t *state);

/**
 * Reset the FPS tracker to the initial state.
 *
 * Clears frame count and FPS value. Useful when restarting measurement.
 *
 * Parameters:
 *   state - Pointer to fps_state_t to reset.
 */
void fps_reset(fps_state_t *state);

/**
 * Record a frame and update FPS calculation if interval elapsed.
 *
 * Call once per frame. FPS is recalculated every second.
 *
 * Parameters:
 *   state - Pointer to fps_state_t.
 *
 * Returns: 1 if FPS was recalculated, 0 otherwise.
 */
extern int fps_update(fps_state_t *state);

/**
 * Get the current FPS value.
 *
 * Parameters:
 *   state - Pointer to fps_state_t.
 *
 * Returns: Current FPS, or 0.0 if not yet calculated.
 */
double fps_get(const fps_state_t *state);

/**
 * Format FPS value as string.
 *
 * Parameters:
 *   state - Pointer to fps_state_t.
 *   buf   - Output buffer.
 *   size  - Size of output buffer.
 *
 * Returns: Number of characters written (excluding null terminator).
 */
extern int fps_format(const fps_state_t *state, char *buf, size_t size);

#endif
