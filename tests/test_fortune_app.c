#include "fortune_app_stubs/app_platform.h"
#include "../main/main.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool locked,quiet_ok=true,quiet,opening,muted,sound_key,volume_key;
static uint8_t saved_sound=1,saved_state[400]; static size_t saved_size;
static uint8_t saved_volume,played_volume;
static unsigned writes,commits,sounds[FORTUNE_SOUND_COUNT];
static bool fail_commit;
void audio_test_log(const char *tag,const char *format,...) { (void)tag; (void)format; }
const char *esp_err_to_name(esp_err_t e) { (void)e; return "injected"; }
uint32_t esp_random(void) { return 42; }
int64_t esp_timer_get_time(void) { return 0; }
esp_err_t nvs_flash_init(void) { return ESP_OK; }
esp_err_t nvs_open(const char *name,int mode,nvs_handle_t *handle) { assert(!strcmp(name,"fortune") && mode==NVS_READWRITE); *handle=1; return ESP_OK; }
esp_err_t nvs_get_blob(nvs_handle_t handle,const char *key,void *data,size_t *size) {
    (void)handle; assert(!strcmp(key,"state"));
    if(!saved_size) return ESP_ERR_NVS_NOT_FOUND;
    assert(*size>=saved_size); memcpy(data,saved_state,saved_size); *size=saved_size; return ESP_OK;
}
esp_err_t nvs_get_u8(nvs_handle_t handle,const char *key,uint8_t *value) {
    (void)handle;
    if(!strcmp(key,"volume")) { if(!volume_key) return ESP_ERR_NVS_NOT_FOUND; *value=saved_volume; }
    else { assert(!strcmp(key,"sound")); if(!sound_key) return ESP_ERR_NVS_NOT_FOUND; *value=saved_sound; }
    return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t handle,const char *key,const void *data,size_t size) {
    (void)handle; assert(!strcmp(key,"state") && quiet && !locked && size<=sizeof(saved_state));
    memcpy(saved_state,data,size); saved_size=size; ++writes; return ESP_OK;
}
esp_err_t nvs_set_u8(nvs_handle_t handle,const char *key,uint8_t value) {
    (void)handle; assert(quiet && !locked);
    if(!strcmp(key,"volume")) { assert(value>=10 && value<=100 && value%10==0); saved_volume=value; volume_key=true; }
    else { assert(!strcmp(key,"sound") && value<=1); saved_sound=value; sound_key=true; }
    ++writes; return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle) { (void)handle; assert(quiet && !locked); ++commits; return fail_commit?ESP_FAIL:ESP_OK; }
bool fortune_audio_quiet(void) { assert(!locked); quiet=quiet_ok; return quiet_ok; }
void fortune_audio_resume(void) { quiet=false; }
void fortune_audio_enable(bool enabled) { muted=!enabled; }
void fortune_audio_volume(uint8_t percent) { played_volume=percent; }
bool fortune_audio_failed(void) { return false; }
void fortune_audio_play(fortune_sound_t sound,unsigned variant) { (void)variant; if(!muted) ++sounds[sound]; }
bool bsp_lvgl_lock(int timeout) { (void)timeout; assert(!locked); locked=true; return true; }
void bsp_lvgl_unlock(void) { assert(locked); locked=false; }
void fortune_ui_sound_enabled(bool enabled) { assert(locked); (void)enabled; }
void fortune_ui_volume(bool visible,unsigned percent,const char *notice) {
    assert(locked); (void)visible; (void)percent; (void)notice;
}
void fortune_ui_update(const fortune_state_t *state,fortune_page_t page,int battery,const char *notice) {
    assert(locked); (void)state; (void)page; (void)battery; (void)notice;
}
bool fortune_ui_revealing(void) { assert(locked); return opening; }
void fortune_ui_begin_reveal(void) { assert(locked); opening=true; fortune_audio_play(FORTUNE_SOUND_OPEN,s_state.current.art); }
void fortune_ui_finish_reveal(void) { assert(locked); if(opening) fortune_audio_play(FORTUNE_SOUND_REVEAL,s_state.current.art); opening=false; }

int main(void) {
    load(); assert(s_sound_enabled && s_storage_ready && s_volume==80); s_page=FORTUNE_HOME;
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(opening && sounds[FORTUNE_SOUND_OPEN]==1);
    unsigned remaining=fortune_remaining(&s_state);
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); assert(fortune_remaining(&s_state)==remaining);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(!opening && sounds[FORTUNE_SOUND_REVEAL]==1);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_SHOWCASE && sounds[FORTUNE_SOUND_KEEP]==1);
    fortune_card_t pin=s_state.pinned;
    process((input_t){BSP_BTN_DOWN,BSP_BTN_LONG});
    assert(!s_sound_enabled && muted && sound_key && saved_sound==0);
    assert(s_state.pinned.quote==pin.quote && s_state.pinned.art==pin.art);
    /* Simulate reset and reload the actual encoded state plus the new key. */
    s_sound_enabled=true; load(); assert(!s_sound_enabled);
    assert(s_state.pinned.quote==pin.quote && s_state.pinned.art==pin.art);
    sound_key=false; s_sound_enabled=true; load(); assert(s_sound_enabled); /* Legacy upgrade. */
    s_page=FORTUNE_HOME; unsigned style=s_state.style;
    process((input_t){BSP_BTN_DOWN,BSP_BTN_LONG}); assert(s_state.style==(style+1)%4);
    unsigned before=writes; quiet_ok=false;
    assert(!store(&s_state)); assert(writes==before && !quiet);
    quiet_ok=true; fail_commit=true; muted=false; s_page=FORTUNE_REVEAL;
    unsigned kept=sounds[FORTUNE_SOUND_KEEP]; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_save_error && sounds[FORTUNE_SOUND_KEEP]==kept); /* No success chime on failed save. */
    fail_commit=false; s_save_error=false; s_page=FORTUNE_SHOWCASE;
    /* Preview is isolated: no deck/card writes until OK; cancel restores mute and volume. */
    s_sound_enabled=false; muted=true; unsigned unread=fortune_remaining(&s_state);
    before=writes;
    process((input_t){BSP_BTN_UP,BSP_BTN_LONG}); assert(s_volume_open && !muted && played_volume==80);
    for(unsigned i=0;i<12;++i) process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK});
    assert(s_volume_candidate==10 && played_volume==10 && writes==before);
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    assert(!s_volume_open && muted && played_volume==80 && writes==before);
    assert(fortune_remaining(&s_state)==unread && s_state.pinned.art==pin.art && s_state.pinned.quote==pin.quote);
    process((input_t){BSP_BTN_UP,BSP_BTN_LONG});
    for(unsigned i=0;i<12;++i) process((input_t){BSP_BTN_UP,BSP_BTN_CLICK});
    assert(s_volume_candidate==100 && writes==before);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(!s_volume_open && s_volume==100 && saved_volume==100 && saved_sound==1 && !muted);
    load(); assert(s_volume==100 && s_sound_enabled); /* Persisted across reset. */
    process((input_t){BSP_BTN_DOWN,BSP_BTN_LONG}); assert(!s_sound_enabled && s_volume==100);
    process((input_t){BSP_BTN_DOWN,BSP_BTN_LONG}); assert(s_sound_enabled && s_volume==100);
    process((input_t){BSP_BTN_UP,BSP_BTN_LONG});
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_volume_candidate==90);
    quiet_ok=false; before=writes; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_volume_open && s_save_error && s_volume==100 && writes==before);
    quiet_ok=true; fail_commit=true; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_volume_open && s_save_error && s_volume==100);
    fail_commit=false; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(!s_volume_open && !s_save_error && s_volume==90 && saved_volume==90);
    for(unsigned bad=0;bad<256;++bad) if(bad<10 || bad>100 || bad%10) {
        saved_volume=(uint8_t)bad; load(); assert(s_volume==80);
    }
    volume_key=false; load(); assert(s_volume==80); /* Upgrade from pre-volume firmware. */
    assert(commits>0);
    puts("Fortune input/storage: PASS (draw/skip/keep, mute, volume preview/save/cancel/bounds/reload, legacy and invalid defaults, unchanged cards, failed-save retry)");
    return 0;
}
