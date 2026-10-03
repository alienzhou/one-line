#pragma once
#include <stdint.h>
#define FORTUNE_PIXEL_W 108
#define FORTUNE_PIXEL_H 58
typedef struct { uint32_t paper, ink, muted, accent; } fortune_colors_t;
/* Original pixel scenes. RGB565, no heap, no LVGL or hardware dependency. */
fortune_colors_t fortune_pixel_colors(uint32_t id);
void fortune_pixels(uint16_t *pixels, uint32_t id, uint32_t frame);
void fortune_pixel_unwrap(uint16_t *pixels, uint32_t id, unsigned step);
void fortune_pixel_wipe(uint16_t *pixels, uint32_t old_id, uint32_t new_id, unsigned step);
