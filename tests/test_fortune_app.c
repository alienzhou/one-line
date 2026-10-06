#include "fortune_app_stubs/app_platform.h"
#include "../main/main.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool locked,quiet_ok=true,quiet,opening,muted,sound_key,volume_key;
static uint8_t saved_sound=1,saved_state[FORTUNE_STATE_BYTES]; static size_t saved_size;
static uint8_t saved_volume,played_volume;
static unsigned writes,commits,sounds[FORTUNE_SOUND_COUNT];
static bool fail_commit;
static uint8_t saved_mail[FORTUNE_MAIL_BYTES],pending_mail[FORTUNE_MAIL_BYTES];
static size_t saved_mail_size,pending_mail_size;
void audio_test_log(const char *tag,const char *format,...) { (void)tag; (void)format; }
const char *esp_err_to_name(esp_err_t e) { (void)e; return "injected"; }
uint32_t esp_random(void) { return 42; }
int64_t esp_timer_get_time(void) { return 0; }
esp_err_t nvs_flash_init(void) { return ESP_OK; }
esp_err_t nvs_open(const char *name,int mode,nvs_handle_t *handle) { assert(!strcmp(name,"fortune") && mode==NVS_READWRITE); *handle=1; return ESP_OK; }
esp_err_t nvs_get_blob(nvs_handle_t handle,const char *key,void *data,size_t *size) {
    (void)handle;
    if (!strcmp(key,"mail")) {
        if (!saved_mail_size) return ESP_ERR_NVS_NOT_FOUND;
        assert(*size>=saved_mail_size); memcpy(data,saved_mail,saved_mail_size); *size=saved_mail_size; return ESP_OK;
    }
    assert(!strcmp(key,"state"));
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
    (void)handle; assert(quiet && !locked);
    if (!strcmp(key,"mail")) {
        assert(size==sizeof(pending_mail)); memcpy(pending_mail,data,size); pending_mail_size=size; ++writes; return ESP_OK;
    }
    assert(!strcmp(key,"state") && size<=sizeof(saved_state));
    memcpy(saved_state,data,size); saved_size=size; ++writes; return ESP_OK;
}
esp_err_t nvs_set_u8(nvs_handle_t handle,const char *key,uint8_t value) {
    (void)handle; assert(quiet && !locked);
    if(!strcmp(key,"volume")) { assert(value>=10 && value<=100 && value%10==0); saved_volume=value; volume_key=true; }
    else { assert(!strcmp(key,"sound") && value<=1); saved_sound=value; sound_key=true; }
    ++writes; return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle) {
    (void)handle; assert(quiet && !locked); ++commits;
    if (fail_commit) { pending_mail_size=0; return ESP_FAIL; }
    if (pending_mail_size) { memcpy(saved_mail,pending_mail,pending_mail_size); saved_mail_size=pending_mail_size; pending_mail_size=0; }
    return ESP_OK;
}
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
void fortune_ui_album(unsigned index,unsigned action,bool confirm) { assert(locked); (void)index; (void)action; (void)confirm; }
void fortune_ui_turn(int direction) { assert(locked); assert(direction==1 || direction==-1); }
void fortune_ui_kept(void) { assert(locked); }
void fortune_ui_mail(const fortune_mail_t *state,const fortune_mail_view_t *view,int battery,const char *notice) {
    assert(locked && fortune_mail_valid(state)); (void)view; (void)battery; (void)notice;
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
    s_page=FORTUNE_HOME; s_state.mood=6; s_state.style=0;
    process((input_t){BSP_BTN_DOWN,BSP_BTN_LONG}); assert(s_state.mood==0 && s_state.style==FORTUNE_ANY_STYLE);
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
    s_page=FORTUNE_HOME; s_state.style=0; s_state.mood=1;
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_TOPICS && s_topic_candidate==0 && s_state.mood==1);
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_topic_candidate==1);
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_topic_candidate==2);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(opening && s_state.mood==2 && FORTUNE_RECORDS[s_state.current.quote].mood==2);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(!opening);
    uint8_t history[FORTUNE_SEEN_BYTES]; memcpy(history,s_state.seen,sizeof(history));
    load(); assert(s_state.mood==0 && !memcmp(history,s_state.seen,sizeof(history)));
    s_page=FORTUNE_REVEAL; process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    assert(s_page==FORTUNE_HOME && s_state.mood==0);
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_topic_candidate==1);
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    assert(s_page==FORTUNE_HOME && s_state.mood==0); /* Cancel never applies hidden preview. */
    process((input_t){BSP_BTN_DOWN,BSP_BTN_LONG}); assert(s_state.mood==0 && s_state.style==FORTUNE_ANY_STYLE);
    process((input_t){BSP_BTN_DOWN,BSP_BTN_LONG}); assert(s_state.style==FORTUNE_ANY_STYLE);
    assert(commits>0);
    /* Exercise all 16 slots using actual draw/skip/collect button events. */
    fortune_defaults(&s_state,777); s_page=FORTUNE_HOME; s_volume_open=false;
    for(unsigned i=0;i<16;++i) {
        process((input_t){i?BSP_BTN_UP:BSP_BTN_OK,BSP_BTN_CLICK});
        process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); /* Skip only. */
        process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
        assert(s_state.favorite_count==i+1 && s_page==FORTUNE_SHOWCASE);
    }
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    fortune_state_t full=s_state;
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_ALBUM_REPLACE);
    before=writes;
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_album_index==1 && writes==before);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_ALBUM_CONFIRM && !s_album_confirm);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_ALBUM_REPLACE);
    assert(!memcmp(&s_state,&full,sizeof(full)) && writes==before);
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG}); assert(s_page==FORTUNE_REVEAL);
    assert(!memcmp(&s_state,&full,sizeof(full)));
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_album_confirm);
    quiet_ok=false; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_ALBUM_CONFIRM && s_save_error && !memcmp(&s_state,&full,sizeof(full)));
    quiet_ok=true; fail_commit=true; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_ALBUM_CONFIRM && !memcmp(&s_state,&full,sizeof(full)));
    fail_commit=false; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_SHOWCASE && !s_save_error && s_state.favorite_count==16);
    assert(s_state.favorites[1].quote==full.current.quote && s_state.pinned.quote==full.current.quote);
    load(); assert(s_state.favorite_count==16 && s_state.favorites[1].quote==full.current.quote);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_ALBUM && s_album_index==1);
    before=writes; process((input_t){BSP_BTN_UP,BSP_BTN_CLICK});
    assert(s_album_index==0 && writes==before);
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); assert(s_album_index==15);
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_album_index==0);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_ALBUM_ACTIONS);
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_ALBUM && s_state.favorite_count==16);
    fortune_card_t changed=s_state.favorites[0];
    assert(changed.quote==full.favorites[0].quote && changed.art!=full.favorites[0].art);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_SHOWCASE && s_state.pinned.quote==changed.quote && s_state.pinned.art==changed.art);
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK});
    assert(s_state.favorites[0].art==s_state.pinned.art); /* Showcase skins stay synchronized. */
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_ALBUM_REMOVE && !s_album_confirm);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_ALBUM && s_state.favorite_count==16);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); process((input_t){BSP_BTN_UP,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_ALBUM && s_state.favorite_count==15 && s_state.pinned.quote==changed.quote);
    before=writes; process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG}); assert(s_page==FORTUNE_ALBUM && writes==before);
    fortune_defaults(&s_state,42); s_page=FORTUNE_HOME;
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG}); assert(s_page==FORTUNE_ALBUM && !s_state.favorite_count);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_page==FORTUNE_HOME);
    puts("Collection input/storage: PASS (16 slots, full preview/cancel/replace, failure retry, restart, wrap, actions, signature/skin sync, remove/cancel, empty album; browsing never writes)");
    puts("Fortune input/storage: PASS (draw/skip/keep, explicit topic preview/confirm/cancel, global default after reload/return, mute, volume preview/save/cancel/bounds, saved cards, failed-save retry)");
    /* A single type selector opens a preview. No bookmark is changed by browsing. */
    assert(store(&s_state));
    fortune_state_t deck=s_state; uint8_t volume=s_volume; bool sound=s_sound_enabled;
    before=writes;
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_page==FORTUNE_TOPICS);
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); assert(s_topic_candidate==FORTUNE_MAIL_TOPIC);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_page==FORTUNE_MAIL && !s_mail.active && !s_mail_view.reading && writes==before);
    unsigned seen=1U<<s_mail_view.story;
    for (unsigned i=1;i<FORTUNE_MAIL_STORIES;++i) {
        unsigned old=s_mail_view.story;
        process((input_t){BSP_BTN_UP,BSP_BTN_CLICK});
        assert(s_mail_view.story!=old && !(seen&(1U<<s_mail_view.story)));
        seen|=1U<<s_mail_view.story;
    }
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_mail_view.style==1 && writes==before);
    unsigned first=s_mail_view.story;
    fail_commit=true; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_mail_error && !s_mail_view.reading && !s_mail.active);
    fail_commit=false; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(!s_mail_error && s_mail_view.reading && s_mail.active && s_mail.selected==first);
    before=writes;
    process((input_t){BSP_BTN_DOWN,BSP_BTN_CLICK}); assert(s_mail_view.page==0 && writes==before);
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); assert(s_mail_view.page==0 && writes==before);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_mail_view.page==1);
    load(); assert(s_mail_view.reading && s_mail_view.story==first && s_mail_view.page==1);
    fail_commit=true; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(s_mail_error && s_mail_view.page==1 && s_mail.bookmarks[first]==2);
    fail_commit=false; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_mail_view.page==2);
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); assert(s_mail_view.page==1);
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG}); assert(s_page==FORTUNE_HOME && !s_mail.active);
    open_mail(); assert(!s_mail_view.reading && s_mail_view.story==first);
    process((input_t){BSP_BTN_UP,BSP_BTN_CLICK}); unsigned second=s_mail_view.story; assert(second!=first);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    for (unsigned i=0;i<3;++i) process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    open_mail();
    while (s_mail_view.story!=first) process((input_t){BSP_BTN_UP,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_mail_view.page==1 && s_mail.bookmarks[second]==4);
    for (unsigned page=2;page<fortune_mail_pages(first);++page) {
        process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_mail_view.page==page);
    }
    fail_commit=true; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_mail_view.reading);
    fail_commit=false; process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    assert(!s_mail_view.reading && !s_mail.active && s_mail.bookmarks[first]==fortune_mail_pages(first)+1);
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_mail_view.reading && s_mail_view.page==0);
    /* The same sound controls work while reading; they do not advance the story. */
    process((input_t){BSP_BTN_UP,BSP_BTN_LONG}); assert(s_volume_open);
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG}); assert(!s_volume_open && s_mail_view.page==0);
    quiet_ok=false; process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    assert(s_page==FORTUNE_HOME && s_mail_error); /* A failed save cannot trap navigation. */
    quiet_ok=true; open_mail(); process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    assert(!s_mail.active);
    /* Invalid records stay intact while allowing visibly unsaved, offline reading. */
    saved_mail[2]^=1; uint8_t corrupt[16]; memcpy(corrupt,saved_mail,16);
    load(); assert(s_mail_corrupt); before=writes;
    open_mail(); process((input_t){BSP_BTN_OK,BSP_BTN_CLICK});
    process((input_t){BSP_BTN_OK,BSP_BTN_CLICK}); assert(s_mail_view.page==1);
    process((input_t){BSP_BTN_OK,BSP_BTN_LONG});
    assert(s_page==FORTUNE_HOME && s_mail_corrupt && writes==before && !memcmp(corrupt,saved_mail,16));
    assert(!memcmp(&s_state,&deck,sizeof(deck)) && s_volume==volume && s_sound_enabled==sound);
    puts("Continuous letters app: PASS (type selection, non-destructive random preview, confirm/resume, fixed down-for-art, linear OK reading, per-story bookmarks, failure retry, corruption preservation, collection isolation)");
    return 0;
}
