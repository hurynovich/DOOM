#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "../include/my_fps.h"

#define FPS_INTERVAL 1.0

struct fps_state {
    struct timespec last_time;
    struct timespec frame_time;
    unsigned long frame_count;
    double current_fps;
    int fps_ready;
};

static double timespec_diff(const struct timespec *a, const struct timespec *b)
{
    return (double) (a->tv_sec - b->tv_sec)
           + (double) (a->tv_nsec - b->tv_nsec) / 1e9;
}

fps_state_t *fps_create(void)
{
    fps_state_t *state = calloc(1, sizeof(fps_state_t));
    if (!state)
        return NULL;

    clock_gettime(CLOCK_MONOTONIC, &state->last_time);
    state->frame_time = state->last_time;
    state->frame_count = 0;
    state->current_fps = 0.0;
    state->fps_ready = 0;

    return state;
}

void fps_destroy(fps_state_t *state)
{
    free(state);
}

void fps_reset(fps_state_t *state)
{
    if (!state)
        return;

    clock_gettime(CLOCK_MONOTONIC, &state->last_time);
    state->frame_time = state->last_time;
    state->frame_count = 0;
    state->current_fps = 0.0;
    state->fps_ready = 0;
}

int fps_update(fps_state_t *state)
{
    if (!state)
        return 0;

    clock_gettime(CLOCK_MONOTONIC, &state->frame_time);
    state->frame_count++;

    double elapsed = timespec_diff(&state->frame_time, &state->last_time);

    if (elapsed >= FPS_INTERVAL) {
        state->current_fps = (double) state->frame_count / elapsed;
        state->last_time = state->frame_time;
        state->frame_count = 0;
        state->fps_ready = 1;
        return 1;
    }

    return 0;
}

double fps_get(const fps_state_t *state)
{
    if (!state)
        return 0.0;
    return state->current_fps;
}

int fps_format(const fps_state_t *state, char *buf, size_t size)
{
    if (!state || !buf || size == 0)
        return 0;

    if (!state->fps_ready) {
        return snprintf(buf, size, "FPS: ...");
    }

    return snprintf(buf, size, "FPS: %.1f", state->current_fps);
}
