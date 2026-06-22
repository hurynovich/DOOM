/*
 * SDL2 Audio Test - Simple Tone Generator
 * =========================================
 *
 * This program demonstrates basic SDL2 audio output by generating a sine wave tone.
 * It's useful for verifying that SDL2 audio works on your system.
 *
 * AUDIO CONCEPTS:
 * ---------------
 * - Sample Rate (44100 Hz): Number of audio samples per second. CD quality is 44.1kHz.
 * - Frequency (220 Hz): Pitch of the tone. 220 Hz is the musical note A3.
 * - Amplitude (28000): Volume/loudness. Range is -32768 to 32767 for 16-bit audio.
 * - Duration (1 second): How long the tone plays.
 * - Channels (2): Stereo audio (left and right).
 * - Format (AUDIO_S16LSB): 16-bit signed integers, little-endian byte order.
 *
 * HOW IT WORKS:
 * -------------
 * 1. Initialize SDL2 audio subsystem
 * 2. Open audio device with desired specifications
 * 3. SDL2 calls our callback function periodically to fill audio buffer
 * 4. Callback generates sine wave samples using sin() function
 * 5. Main thread waits for duration, then closes audio device
 *
 * COMPILE:
 * --------
 * gcc -o sdl_audio_test sdl_audio_test.c -lSDL2 -lm
 *
 * RUN:
 * ----
 * ./sdl_audio_test
 *
 * EXPECTED OUTPUT:
 * ----------------
 * You should hear a 220 Hz (A3 note) tone for 1 second through your speakers.
 * Console will show: "Playing 220 Hz tone for 1 seconds..." then "Done."
 *
 * TROUBLESHOOTING:
 * ----------------
 * - If you get "SDL_Init failed": Check that SDL2 is installed
 * - If you get "SDL_OpenAudioDevice failed": Check audio device availability
 * - If no sound: Check system volume, audio output device, and permissions
 * - For PulseAudio issues: Try "pasuspender -- ./sdl_audio_test"
 */

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define SAMPLE_RATE 44100
#define DURATION 10
#define FREQUENCY 80.0
#define AMPLITUDE 8000

static double phase = 0.0;

/*
 * Audio Callback Function
 * -----------------------
 * Called by SDL2 when it needs more audio data to play.
 * 
 * Parameters:
 *   userdata - Custom data pointer (unused here, passed as NULL)
 *   stream   - Buffer to fill with audio samples
 *   len      - Length of buffer in bytes
 *
 * The buffer contains interleaved stereo samples:
 *   [L0, R0, L1, R1, L2, R2, ...]
 * where each sample is a 16-bit signed integer (2 bytes).
 */
void audio_callback(void *userdata, Uint8 *stream, int len) {
    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2;  // Convert bytes to number of samples
    static int freq = FREQUENCY;
    static int direction = 1;
    freq += direction * 20;
    direction *= (freq > 600 || freq < 60) ? -1 : 1;

    double phase_step = 2.0 * M_PI * freq / SAMPLE_RATE;

    // Generate samples for both channels (stereo)
    for (int i = 0; i < length; i += 2) {
        // Calculate sine wave sample value
        Sint16 sample = (Sint16)(sin(phase) * AMPLITUDE);
        
        // Write same sample to left and right channels
        buffer[i] = sample;      // Left channel
        buffer[i + 1] = sample;  // Right channel
        
        // Advance phase for next sample
        phase += phase_step;
        if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
    }
}

int main(int argc, char **argv) {
    // Initialize SDL2 audio subsystem
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Configure desired audio format
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = SAMPLE_RATE;      // 44100 Hz sample rate
    desired.format = AUDIO_S16LSB;   // 16-bit signed, little-endian
    desired.channels = 2;            // Stereo
    desired.samples = 1024;          // Buffer size (affects latency)
    desired.callback = audio_callback;

    // Open the default audio device
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if (dev == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    fprintf(stderr, "Playing %d Hz tone for %d seconds...\n", (int)FREQUENCY, DURATION);
    
    // Start audio playback (unpause the device)
    SDL_PauseAudioDevice(dev, 0);
    
    // Wait for the specified duration
    SDL_Delay(DURATION * 1000);
    
    // Clean up
    SDL_CloseAudioDevice(dev);
    SDL_Quit();

    fprintf(stderr, "Done.\n");
    return 0;
}
