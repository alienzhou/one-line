/* Original application. The input worker owns state and all slow operations. */
#include "fortune_model.h"
#include "fortune_ui.h"
#include "fortune_text.h"
#include "fortune_audio.h"
#include "bsp_display.h"
#include "bsp_button.h"
#include "bsp_battery.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "fortune";
static fortune_state_t s_state;
static fortune_page_t s_page;
static QueueHandle_t s_inputs;
static nvs_handle_t s_nvs;
static bool s_storage_ready, s_save_error, s_input_error, s_battery_ready;
static int s_battery = -1;
static const char *s_notice;
static bool s_unwrap_requested;
static bool s_sound_enabled=true;
static uint8_t s_volume=FORTUNE_VOLUME_DEFAULT, s_volume_candidate;
static bool s_volume_open;
static uint8_t s_topic_candidate;
static uint8_t s_album_index, s_album_action;
static bool s_album_confirm, s_keep_feedback;
static int s_turn_direction;
static uint8_t s_storage_bytes[FORTUNE_STATE_BYTES];
typedef struct { bsp_btn_t key; bsp_btn_ev_t event; } input_t;

static bool store(const fortune_state_t *state) {
    if (!s_storage_ready) return false;
    if (!fortune_audio_quiet()) { fortune_audio_resume(); return false; }
    size_t n = fortune_encode_state(state, s_storage_bytes, sizeof(s_storage_bytes));
    esp_err_t err = n ? nvs_set_blob(s_nvs, "state", s_storage_bytes, n) : ESP_ERR_INVALID_ARG;
    if (err == ESP_OK) err = nvs_set_u8(s_nvs, "sound", s_sound_enabled?1:0);
    if (err == ESP_OK) err = nvs_set_u8(s_nvs, "volume", s_volume);
    if (err == ESP_OK) err = nvs_commit(s_nvs);
    fortune_audio_resume();
    if (err != ESP_OK) ESP_LOGE(TAG, "Save failed: %s", esp_err_to_name(err));
    return err == ESP_OK;
}

static void load(void) {
    s_sound_enabled=true; s_volume=FORTUNE_VOLUME_DEFAULT;
    fortune_defaults(&s_state, esp_random() ^ (uint32_t)esp_timer_get_time());
    esp_err_t err = nvs_flash_init();
    /* Do not erase existing user data to recover this app's storage. */
    if (err == ESP_OK) err = nvs_open("fortune", NVS_READWRITE, &s_nvs);
    s_storage_ready = err == ESP_OK;
    if (!s_storage_ready) { s_save_error = true; return; }
    uint8_t sound=1;
    if (nvs_get_u8(s_nvs,"sound",&sound)==ESP_OK && sound<=1) s_sound_enabled=sound!=0;
    uint8_t volume=FORTUNE_VOLUME_DEFAULT;
    if (nvs_get_u8(s_nvs,"volume",&volume)==ESP_OK && volume>=FORTUNE_VOLUME_MIN &&
        volume<=FORTUNE_VOLUME_MAX && volume%FORTUNE_VOLUME_STEP==0) s_volume=volume;
    size_t n = sizeof(s_storage_bytes);
    err = nvs_get_blob(s_nvs, "state", s_storage_bytes, &n);
    if (err == ESP_ERR_NVS_NOT_FOUND) return;
    if (err != ESP_OK || !fortune_decode_state(&s_state, s_storage_bytes, n)) {
        ESP_LOGW(TAG, "State unavailable or corpus changed; using new deck");
        s_notice = "旧存档不兼容，已使用新签库";
    }
    /* A restored topic must not silently restrict the default draw mode. */
    fortune_select_topic(&s_state,0);
}

static void render(void) {
    if (!bsp_lvgl_lock(1000)) return;
    fortune_ui_sound_enabled(s_volume_open || s_sound_enabled);
    fortune_ui_album(s_album_index,s_album_action,s_album_confirm);
    fortune_state_t preview=s_state;
    if(s_page==FORTUNE_TOPICS) preview.mood=s_topic_candidate;
    fortune_ui_update(&preview, s_page, s_battery,
        s_input_error ? FT_INPUT_ERROR : s_save_error ? FT_SAVE_ERROR :
        s_sound_enabled && fortune_audio_failed() ? FT_SOUND_ERROR : s_notice);
    fortune_ui_volume(s_volume_open,s_volume_candidate,
        s_save_error?FT_SAVE_ERROR:fortune_audio_failed()?FT_SOUND_ERROR:NULL);
    if (s_unwrap_requested) { fortune_ui_begin_reveal(); s_unwrap_requested = false; }
    if (s_turn_direction) { fortune_ui_turn(s_turn_direction); s_turn_direction=0; }
    if (s_keep_feedback) { fortune_ui_kept(); s_keep_feedback=false; }
    bsp_lvgl_unlock();
}

static void save_after_change(void) {
    s_save_error = !store(&s_state);
}

static void return_home(void) {
    bool filtered=s_state.mood!=0;
    fortune_select_topic(&s_state,0); s_topic_candidate=0; s_page=FORTUNE_HOME;
    if(filtered) save_after_change();
}

static bool commit_candidate(const fortune_state_t *candidate) {
    s_save_error=!store(candidate);
    if (s_save_error) return false;
    s_state=*candidate;
    return true;
}

static void open_album(void) {
    int pin=fortune_favorite_find(&s_state,s_state.pinned.quote);
    s_album_index=pin>=0?(uint8_t)pin:0;
    s_album_action=0; s_album_confirm=false; s_page=FORTUNE_ALBUM;
}

static void collect_current(unsigned slot) {
    fortune_state_t candidate=s_state;
    bool existed=fortune_favorite_find(&s_state,s_state.current.quote)>=0;
    if (!fortune_favorite_save(&candidate,candidate.current,slot)) return;
    candidate.pinned=candidate.current;
    if (!commit_candidate(&candidate)) return;
    s_page=FORTUNE_SHOWCASE; s_keep_feedback=true;
    s_notice=existed?FT_ALBUM_UPDATED:FT_ALBUM_SAVED;
    fortune_audio_play(FORTUNE_SOUND_KEEP,s_state.pinned.art);
}

static bool album_page(void) { return s_page>=FORTUNE_ALBUM && s_page<=FORTUNE_ALBUM_CONFIRM; }

static void album_input(input_t in) {
    if (in.event==BSP_BTN_LONG && in.key==BSP_BTN_OK) {
        if (s_page==FORTUNE_ALBUM_ACTIONS || s_page==FORTUNE_ALBUM_REMOVE) s_page=FORTUNE_ALBUM;
        else if (s_page==FORTUNE_ALBUM_CONFIRM) s_page=FORTUNE_ALBUM_REPLACE;
        else if (s_page==FORTUNE_ALBUM_REPLACE) s_page=FORTUNE_REVEAL;
        else return_home();
        s_album_confirm=false;
    } else if (in.event==BSP_BTN_CLICK && in.key!=BSP_BTN_OK) {
        int direction=in.key==BSP_BTN_UP?-1:1;
        if (s_page==FORTUNE_ALBUM_REMOVE || s_page==FORTUNE_ALBUM_CONFIRM)
            s_album_confirm=!s_album_confirm;
        else if (s_page==FORTUNE_ALBUM_ACTIONS)
            s_album_action=(uint8_t)((s_album_action+(direction<0?2:1))%3);
        else if (s_state.favorite_count>1) {
            s_album_index=(uint8_t)((s_album_index+s_state.favorite_count+direction)%s_state.favorite_count);
            s_turn_direction=direction;
        }
    } else if (in.event==BSP_BTN_CLICK && in.key==BSP_BTN_OK) {
        if (s_page==FORTUNE_ALBUM) {
            if (s_state.favorite_count) { s_page=FORTUNE_ALBUM_ACTIONS; s_album_action=0; }
            else return_home();
        } else if (s_page==FORTUNE_ALBUM_REPLACE) {
            s_page=FORTUNE_ALBUM_CONFIRM; s_album_confirm=false;
        } else if (s_page==FORTUNE_ALBUM_ACTIONS) {
            if (s_album_action==0) {
                fortune_state_t candidate=s_state; candidate.pinned=candidate.favorites[s_album_index];
                if (commit_candidate(&candidate)) {
                    s_page=FORTUNE_SHOWCASE; s_notice=FT_ALBUM_PINNED; s_keep_feedback=true;
                    fortune_audio_play(FORTUNE_SOUND_KEEP,s_state.pinned.art);
                }
            } else if (s_album_action==1) {
                fortune_state_t candidate=s_state;
                fortune_card_t *card=&candidate.favorites[s_album_index]; fortune_remix(&candidate,card);
                if (candidate.pinned.quote==card->quote) candidate.pinned.art=card->art;
                if (commit_candidate(&candidate)) {
                    s_page=FORTUNE_ALBUM;
                    fortune_audio_play(FORTUNE_SOUND_SKIN,card->art);
                }
            } else { s_page=FORTUNE_ALBUM_REMOVE; s_album_confirm=false; }
        } else if (!s_album_confirm) {
            s_page=s_page==FORTUNE_ALBUM_REMOVE?FORTUNE_ALBUM:FORTUNE_ALBUM_REPLACE;
        } else if (s_page==FORTUNE_ALBUM_CONFIRM) collect_current(s_album_index);
        else if (s_page==FORTUNE_ALBUM_REMOVE) {
            fortune_state_t candidate=s_state;
            if (fortune_favorite_remove(&candidate,s_album_index) && commit_candidate(&candidate)) {
                if (s_album_index>=s_state.favorite_count) s_album_index=s_state.favorite_count?s_state.favorite_count-1:0;
                s_page=FORTUNE_ALBUM; s_notice=FT_ALBUM_REMOVED;
            }
        }
    }
    render();
}

static void preview_volume(void) {
    fortune_audio_volume(s_volume_candidate);
    fortune_audio_enable(true);
    fortune_card_t card=album_page() && s_state.favorite_count?s_state.favorites[s_album_index]:
        s_page==FORTUNE_SHOWCASE?s_state.pinned:s_state.current;
    fortune_audio_play(FORTUNE_SOUND_SKIN,card.art);
}

static void volume_input(input_t in) {
    if(in.event==BSP_BTN_LONG && in.key==BSP_BTN_OK) {
        s_volume_open=false;
        fortune_audio_enable(false); /* Cancel the preview even if sound was enabled. */
        fortune_audio_volume(s_volume); fortune_audio_enable(s_sound_enabled);
    } else if(in.event==BSP_BTN_CLICK && in.key==BSP_BTN_OK) {
        uint8_t previous=s_volume; bool enabled=s_sound_enabled;
        s_volume=s_volume_candidate; s_sound_enabled=true;
        save_after_change();
        if(s_save_error) { s_volume=previous; s_sound_enabled=enabled; }
        else { s_volume_open=false; s_notice=FT_VOLUME_SAVED; preview_volume(); }
    } else if(in.event==BSP_BTN_CLICK) {
        if(in.key==BSP_BTN_UP && s_volume_candidate<FORTUNE_VOLUME_MAX)
            s_volume_candidate+=FORTUNE_VOLUME_STEP;
        else if(in.key==BSP_BTN_DOWN && s_volume_candidate>FORTUNE_VOLUME_MIN)
            s_volume_candidate-=FORTUNE_VOLUME_STEP;
        preview_volume();
    }
    render();
}

static void process(input_t in) {
    if (bsp_lvgl_lock(1000)) {
        bool opening = fortune_ui_revealing();
        if (opening && in.key == BSP_BTN_OK) fortune_ui_finish_reveal();
        bsp_lvgl_unlock();
        if (opening) return; /* A repeated key must not consume another unseen record. */
    } else return;
    s_notice = NULL;
    if(s_volume_open) { volume_input(in); return; }
    if (album_page() && !(in.event==BSP_BTN_LONG && in.key!=BSP_BTN_OK)) { album_input(in); return; }
    if (in.event == BSP_BTN_LONG) {
        if (in.key == BSP_BTN_OK) {
            if (s_page != FORTUNE_HOME) return_home();
            else open_album();
        } else if (in.key == BSP_BTN_DOWN && (s_page == FORTUNE_HOME || s_page==FORTUNE_TOPICS)) {
            return_home();
        } else if (in.key == BSP_BTN_DOWN) {
            s_sound_enabled=!s_sound_enabled;
            fortune_audio_enable(s_sound_enabled); save_after_change();
            s_notice=s_sound_enabled?FT_SOUND_ON:FT_SOUND_OFF;
            if(s_sound_enabled) {
                fortune_card_t card=album_page() && s_state.favorite_count?s_state.favorites[s_album_index]:
                    s_page==FORTUNE_SHOWCASE?s_state.pinned:s_state.current;
                fortune_audio_play(FORTUNE_SOUND_SKIN,card.art);
            }
        } else if (in.key == BSP_BTN_UP && s_page != FORTUNE_HOME && s_page!=FORTUNE_TOPICS) {
            s_volume_open=true; s_volume_candidate=s_volume; preview_volume();
        } else if (in.key == BSP_BTN_UP && fortune_remaining(&s_state) == 0) {
            fortune_reset_deck(&s_state); fortune_select_topic(&s_state,0);
            s_topic_candidate=0; save_after_change();
            s_page = FORTUNE_HOME; s_notice = "已重新洗牌";
        }
        render(); return;
    }
    if (in.event != BSP_BTN_CLICK) return;
    if (s_page == FORTUNE_HOME && in.key != BSP_BTN_OK) {
        s_topic_candidate=0; s_page=FORTUNE_TOPICS;
    } else if(s_page==FORTUNE_TOPICS && in.key!=BSP_BTN_OK) {
        s_topic_candidate=(s_topic_candidate+(in.key==BSP_BTN_UP?8:1))%9;
    } else if (((s_page == FORTUNE_HOME || s_page==FORTUNE_TOPICS) && in.key == BSP_BTN_OK) ||
               (s_page != FORTUNE_HOME && in.key == BSP_BTN_UP)) {
        if(s_page==FORTUNE_HOME) fortune_select_topic(&s_state,0);
        else if(s_page==FORTUNE_TOPICS) fortune_select_topic(&s_state,s_topic_candidate);
        if (fortune_draw(&s_state)) {
            save_after_change(); /* Commit seen state before reveal; never store while holding LVGL lock. */
            s_page = FORTUNE_REVEAL;
            s_unwrap_requested = true;
        } else {
            s_topic_candidate=s_state.mood;
            s_page=s_state.mood?FORTUNE_TOPICS:FORTUNE_HOME;
            s_notice = FT_EXHAUSTED_HINT;
        }
    } else if (in.key == BSP_BTN_DOWN) {
        fortune_state_t candidate=s_state;
        fortune_card_t *card=s_page==FORTUNE_SHOWCASE?&candidate.pinned:&candidate.current;
        fortune_remix(&candidate,card);
        int favorite=fortune_favorite_find(&candidate,card->quote);
        if (s_page==FORTUNE_SHOWCASE && favorite>=0) candidate.favorites[favorite].art=card->art;
        if (commit_candidate(&candidate)) fortune_audio_play(FORTUNE_SOUND_SKIN,card->art);
    } else if (in.key == BSP_BTN_OK) {
        if (s_page == FORTUNE_REVEAL) {
            if (s_state.favorite_count==FORTUNE_FAVORITE_CAPACITY &&
                fortune_favorite_find(&s_state,s_state.current.quote)<0) {
                s_album_index=0; s_page=FORTUNE_ALBUM_REPLACE;
            } else collect_current(s_state.favorite_count);
        } else if (s_page==FORTUNE_SHOWCASE) open_album();
        else return_home();
    }
    render();
}

static void worker(void *argument) {
    (void)argument;
    int64_t last_input = esp_timer_get_time(), last_battery = last_input;
    uint8_t brightness = 75;
    bool audio_failed=fortune_audio_failed();
    input_t in;
    for (;;) {
        if (xQueueReceive(s_inputs, &in, pdMS_TO_TICKS(100)) == pdTRUE) {
            bool wake_only = brightness == 0;
            last_input = esp_timer_get_time();
            if (brightness != 75) { bsp_display_backlight(75); brightness = 75; }
            if (!wake_only) process(in);
        }
        int64_t now = esp_timer_get_time();
        if(audio_failed!=fortune_audio_failed()) { audio_failed=fortune_audio_failed(); render(); }
        if (now-last_battery >= 30000000) {
            last_battery = now; s_battery = s_battery_ready ? bsp_battery_soc() : -1; render();
        }
        uint8_t target = now-last_input >= 120000000 ? 0 : now-last_input >= 45000000 ? 15 : 75;
        if (bsp_lvgl_lock(100)) { fortune_ui_motion(target == 75); bsp_lvgl_unlock(); }
        if (target != brightness) { bsp_display_backlight(target); brightness = target; }
    }
}

static void on_key(bsp_btn_t key, bsp_btn_ev_t event, void *user) {
    (void)user;
    if (!s_inputs || (event != BSP_BTN_CLICK && event != BSP_BTN_LONG)) return;
    input_t in = {key, event};
    (void)xQueueSend(s_inputs, &in, 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "Fortune collection: %u complete records, %u procedural appearances",
             FORTUNE_COUNT, FORTUNE_ART_COUNT);
    load();
    s_page = s_state.pinned.quote != FORTUNE_NO_CARD ? FORTUNE_SHOWCASE : FORTUNE_HOME;
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) { ESP_LOGE(TAG, "Display init failed"); return; }
    if (!bsp_lvgl_lock(1000)) return;
    bool created = fortune_ui_create();
    bsp_lvgl_unlock();
    if (!created) { ESP_LOGE(TAG, "UI allocation or glyph inventory failed"); return; }
    s_battery_ready = bsp_battery_init() == ESP_OK;
    s_battery = s_battery_ready ? bsp_battery_soc() : -1;
    (void)fortune_audio_start(s_sound_enabled);
    fortune_audio_volume(s_volume);
    if(bsp_lvgl_lock(1000)) { fortune_ui_sound_callback(fortune_audio_play); bsp_lvgl_unlock(); }
    s_inputs = xQueueCreate(8, sizeof(input_t));
    if (!s_inputs || bsp_button_init(on_key, NULL) != ESP_OK) s_input_error = true;
    render(); bsp_display_backlight(75);
    if (s_inputs && xTaskCreate(worker, "fortune_input", 4096, NULL, 4, NULL) != pdPASS) {
        s_input_error = true; render(); bsp_display_backlight(15);
    }
    ESP_LOGI(TAG, "Free heap=%u largest=%u", (unsigned)esp_get_free_heap_size(),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}
