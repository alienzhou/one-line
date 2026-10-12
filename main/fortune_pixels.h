#pragma once
#include <stdint.h>
#include "fortune_art.h"
#define FORTUNE_PIXEL_W 108
#define FORTUNE_PIXEL_H 58
#define FORTUNE_ARRIVAL_STEPS 16U
/* A two-second authored scene after the envelope opens (125 ms per step).
 * The final step is exactly the saved original's ambient frame zero. */
void fortune_pixel_arrival(uint16_t *pixels, uint32_t id, unsigned step);
typedef struct { uint32_t paper, ink, muted, accent; } fortune_colors_t;
/* Original pixel scenes. RGB565, no heap, no LVGL or hardware dependency. */
fortune_colors_t fortune_pixel_colors(uint32_t id);
uint32_t fortune_skin_index(uint32_t id);
uint32_t fortune_scene_index(uint32_t id);
const char *fortune_scene_name(uint32_t id);
const char *fortune_family_name(unsigned family);
void fortune_pixels(uint16_t *pixels, uint32_t id, uint32_t frame);
void fortune_pixel_unwrap(uint16_t *pixels, uint32_t id, unsigned step);
void fortune_pixel_wipe(uint16_t *pixels, uint32_t old_id, uint32_t new_id, unsigned step);
