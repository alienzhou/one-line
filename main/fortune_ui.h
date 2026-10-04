#pragma once
#include "fortune_model.h"
#include "fortune_sound.h"
#include <stdbool.h>
bool fortune_ui_create(void);
void fortune_ui_update(const fortune_state_t *state, fortune_page_t page,
                       int battery, const char *notice);
/* Every caller is on the LVGL task or owns bsp_lvgl_lock(). */
bool fortune_ui_fonts_valid(void);

void fortune_ui_tick(void);
void fortune_ui_motion(bool enabled);
void fortune_ui_begin_reveal(void);
bool fortune_ui_revealing(void);
void fortune_ui_finish_reveal(void);
/* Callback must only post an event; it is invoked under the LVGL lock. */
void fortune_ui_sound_callback(void (*callback)(fortune_sound_t, unsigned));
void fortune_ui_sound_enabled(bool enabled);
/* Uses the current card's artwork; call after fortune_ui_update under the lock. */
void fortune_ui_volume(bool visible,unsigned percent,const char *notice);
