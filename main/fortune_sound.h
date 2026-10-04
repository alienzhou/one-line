#pragma once
#include <stddef.h>
#include <stdint.h>

#define FORTUNE_SOUND_HZ 16000U
#define FORTUNE_SOUND_VARIANTS 4U
typedef enum {
    FORTUNE_SOUND_NONE, FORTUNE_SOUND_OPEN, FORTUNE_SOUND_REVEAL,
    FORTUNE_SOUND_KEEP, FORTUNE_SOUND_SKIN, FORTUNE_SOUND_COUNT
} fortune_sound_t;

/* Original scores, synthesized as bounded, allocation-free 16-bit mono PCM.
 * The same renderer is used by the device and the audition WAV exporter. */
uint32_t fortune_sound_samples(fortune_sound_t sound);
size_t fortune_sound_render(fortune_sound_t sound, unsigned variant,
                            uint32_t offset, int16_t *pcm, size_t capacity);
