#pragma once
#include "fortune_sound.h"
#include <stdbool.h>

#define FORTUNE_VOLUME_DEFAULT 80U
#define FORTUNE_VOLUME_MIN 10U
#define FORTUNE_VOLUME_MAX 100U
#define FORTUNE_VOLUME_STEP 10U

/* One application-lifetime worker owns all BSP audio calls. These event APIs
 * never wait or access LVGL. A single-slot mailbox replaces stale effects. */
bool fortune_audio_start(bool enabled);
void fortune_audio_play(fortune_sound_t sound, unsigned variant);
void fortune_audio_enable(bool enabled);
/* Applies at the next cue boundary; BSP access stays in the audio worker. */
void fortune_audio_volume(uint8_t percent);
bool fortune_audio_failed(void);
/* Only the input/storage worker may use this pair; never hold the LVGL lock.
 * Prevent PCM/cache stalls around NVS writes. A failed handshake forbids a save. */
bool fortune_audio_quiet(void);
void fortune_audio_resume(void);
