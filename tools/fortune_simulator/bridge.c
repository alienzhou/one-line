/* Desktop I/O adapter. Includes the unchanged firmware event processor/load/store. */
#include "app_platform.h"
#include "../../main/main.c"
#include "lvgl.h"
#include "bsp_display_rounding.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t pixels[240*320];
static uint8_t draw_buffer[240*40*2], saved_state[FORTUNE_STATE_BYTES];
static size_t saved_size;
static uint8_t saved_mail[FORTUNE_MAIL_BYTES];
static size_t saved_mail_size;
static uint8_t saved_sound=1,saved_volume=80;
static const char *storage_path,*frame_path,*wave_path;
static uint32_t random_seed,clock_ms,frame_id,sound_id;
static unsigned audio_volume=80;
static bool audio_enabled=true,locked,drawn;
static fortune_sound_t last_sound;

void audio_test_log(const char *tag,const char *format,...) {
    va_list args; va_start(args,format); fprintf(stderr,"[%s] ",tag);
    vfprintf(stderr,format,args); fputc('\n',stderr); va_end(args);
}
const char *esp_err_to_name(esp_err_t error) { return error==ESP_OK?"OK":"desktop I/O failure"; }
uint32_t esp_random(void) { return random_seed; }
int64_t esp_timer_get_time(void) { return (int64_t)clock_ms*1000; }
size_t esp_get_free_heap_size(void) { return 0; }
size_t heap_caps_get_largest_free_block(unsigned capability) { (void)capability; return 0; }
esp_err_t bsp_display_init(void) { return ESP_OK; }
esp_err_t bsp_battery_init(void) { return ESP_OK; }
int bsp_battery_soc(void) { return 75; } /* Clearly labeled simulated battery in web UI. */
void bsp_display_backlight(uint8_t level) { (void)level; }
bool bsp_lvgl_lock(int timeout) { (void)timeout; if(locked) abort(); locked=true; return true; }
void bsp_lvgl_unlock(void) { if(!locked) abort(); locked=false; }
esp_err_t bsp_button_init(void (*callback)(bsp_btn_t,bsp_btn_ev_t,void *),void *user) {
    (void)callback; (void)user; return ESP_OK;
}
QueueHandle_t xQueueCreate(unsigned length,size_t size) { (void)length; (void)size; return (QueueHandle_t)&clock_ms; }
BaseType_t xQueueSend(QueueHandle_t queue,const void *item,TickType_t ticks) {
    (void)queue; (void)item; (void)ticks; return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t queue,void *item,TickType_t wait) {
    (void)queue; (void)item; (void)wait; return pdFALSE;
}
BaseType_t xTaskCreate(void (*entry)(void *),const char *name,unsigned stack,void *argument,
                       unsigned priority,TaskHandle_t *handle) {
    (void)entry; (void)name; (void)stack; (void)argument; (void)priority; (void)handle;
    return pdPASS; /* The desktop event loop invokes process() on this sole thread. */
}

static void flush(lv_display_t *display,const lv_area_t *area,uint8_t *data) {
    uint16_t *src=(uint16_t *)data;
    for(int y=area->y1;y<=area->y2;++y) {
        int32_t left,right;
        bool visible=bsp_display_rounded_row_span(y,240,320,30,&left,&right);
        for(int x=area->x1;x<=area->x2;++x) {
            uint16_t c=*src++;
            pixels[y*240+x]=visible && x>=left && x<=right?c:0;
        }
    }
    lv_display_flush_ready(display);
}
bool bsp_lvgl_init(void) {
    lv_display_t *d=lv_display_create(240,320);
    if(!d) return false;
    lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(d,draw_buffer,NULL,sizeof(draw_buffer),LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(d,flush); return true;
}
static void put_le(uint8_t *p,uint32_t n,unsigned bytes) {
    for(unsigned i=0;i<bytes;++i) p[i]=(uint8_t)(n>>(i*8));
}
static void write_frame(void) {
    lv_obj_update_layout(lv_screen_active()); lv_refr_now(NULL);
    FILE *f=fopen(frame_path,"wb"); if(!f) { perror(frame_path); exit(1); }
    uint8_t header[54]={0}; header[0]='B'; header[1]='M';
    put_le(header+2,54+240*320*3,4); put_le(header+10,54,4); put_le(header+14,40,4);
    put_le(header+18,240,4); put_le(header+22,320,4); put_le(header+26,1,2); put_le(header+28,24,2);
    fwrite(header,1,sizeof(header),f);
    for(int y=319;y>=0;--y) for(int x=0;x<240;++x) {
        uint16_t c=pixels[y*240+x];
        uint8_t rgb[]={(uint8_t)((c&31)*255/31),(uint8_t)(((c>>5)&63)*255/63),(uint8_t)(((c>>11)&31)*255/31)};
        fwrite(rgb,1,3,f);
    }
    if(fclose(f)) exit(1); ++frame_id;
}
esp_err_t nvs_flash_init(void) {
    FILE *f=fopen(storage_path,"rb"); saved_size=0; saved_mail_size=0;
    if(f) {
        uint8_t header[8];
        if(fread(header,1,8,f)==8 && (!memcmp(header,"FSIM01",6) || !memcmp(header,"FSIM02",6))) {
            saved_sound=header[6]; saved_volume=header[7];
            if (header[5]=='2') {
                saved_mail_size=fread(saved_mail,1,sizeof(saved_mail),f);
                bool present=false;
                for (unsigned i=0;i<saved_mail_size;++i) present|=saved_mail[i]!=0;
                if (!present) saved_mail_size=0;
            }
            saved_size=fread(saved_state,1,sizeof(saved_state),f);
        }
        fclose(f);
    }
    return ESP_OK;
}
esp_err_t nvs_open(const char *name,int mode,nvs_handle_t *handle) {
    (void)name; (void)mode; *handle=1; return ESP_OK;
}
esp_err_t nvs_get_blob(nvs_handle_t handle,const char *key,void *data,size_t *size) {
    (void)handle;
    if (!strcmp(key,"mail")) {
        if (!saved_mail_size) return ESP_ERR_NVS_NOT_FOUND;
        if (*size<saved_mail_size) return ESP_ERR_INVALID_ARG;
        memcpy(data,saved_mail,saved_mail_size); *size=saved_mail_size; return ESP_OK;
    }
    if(!saved_size) return ESP_ERR_NVS_NOT_FOUND;
    if(*size<saved_size) return ESP_ERR_INVALID_ARG;
    memcpy(data,saved_state,saved_size); *size=saved_size; return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t handle,const char *key,const void *data,size_t size) {
    (void)handle;
    if (!strcmp(key,"mail")) {
        if (size!=sizeof(saved_mail)) return ESP_ERR_INVALID_ARG;
        memcpy(saved_mail,data,size); saved_mail_size=size; return ESP_OK;
    }
    if(size>sizeof(saved_state)) return ESP_ERR_INVALID_ARG;
    memcpy(saved_state,data,size); saved_size=size; return ESP_OK;
}
esp_err_t nvs_get_u8(nvs_handle_t handle,const char *key,uint8_t *value) {
    (void)handle; if(!saved_size) return ESP_ERR_NVS_NOT_FOUND;
    *value=!strcmp(key,"sound")?saved_sound:saved_volume; return ESP_OK;
}
esp_err_t nvs_set_u8(nvs_handle_t handle,const char *key,uint8_t value) {
    (void)handle; if(!strcmp(key,"sound")) saved_sound=value; else saved_volume=value; return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle) {
    (void)handle; FILE *f=fopen(storage_path,"wb"); if(!f) return ESP_FAIL;
    uint8_t header[8]={'F','S','I','M','0','2',saved_sound,saved_volume};
    bool ok=fwrite(header,1,8,f)==8 && fwrite(saved_mail,1,sizeof(saved_mail),f)==sizeof(saved_mail) &&
        fwrite(saved_state,1,saved_size,f)==saved_size;
    if(fclose(f)) ok=false; return ok?ESP_OK:ESP_FAIL;
}
bool fortune_audio_start(bool enabled) { audio_enabled=enabled; return true; }
void fortune_audio_enable(bool enabled) { audio_enabled=enabled; }
void fortune_audio_volume(uint8_t percent) { audio_volume=percent; }
bool fortune_audio_quiet(void) { return true; }
void fortune_audio_resume(void) {}
bool fortune_audio_failed(void) { return false; }
void fortune_audio_play(fortune_sound_t sound,unsigned variant) {
    if(!audio_enabled || sound==FORTUNE_SOUND_NONE) return;
    FILE *f=fopen(wave_path,"wb"); if(!f) return;
    uint32_t length=fortune_sound_samples(sound)*2;
    uint8_t header[44]={0}; memcpy(header,"RIFF",4); put_le(header+4,36+length,4);
    memcpy(header+8,"WAVEfmt ",8); put_le(header+16,16,4); put_le(header+20,1,2);
    put_le(header+22,1,2); put_le(header+24,FORTUNE_SOUND_HZ,4);
    put_le(header+28,FORTUNE_SOUND_HZ*2,4); put_le(header+32,2,2); put_le(header+34,16,2);
    memcpy(header+36,"data",4); put_le(header+40,length,4); fwrite(header,1,44,f);
    int16_t pcm[256]; size_t n; uint32_t offset=0;
    while((n=fortune_sound_render(sound,variant,offset,pcm,256))) {
        for(size_t i=0;i<n;++i) { uint8_t b[2]; put_le(b,(uint16_t)pcm[i],2); fwrite(b,1,2,f); }
        offset+=(uint32_t)n;
    }
    fclose(f); last_sound=sound; ++sound_id;
}
static void advance(unsigned ms) {
    for(unsigned n=0;n<ms;) {
        unsigned step=ms-n<25?ms-n:25; n+=step; clock_ms+=step;
        lv_tick_inc(step); lv_timer_handler();
    }
}
static void key(bsp_btn_t button,bsp_btn_ev_t event) {
    uint32_t old=s_state.current.quote;
    process((input_t){button,event});
    if(s_state.current.quote!=old && s_state.current.quote<FORTUNE_COUNT) drawn=true;
}
static void json_string(const char *text) {
    putchar('"');
    for(const unsigned char *p=(const unsigned char *)text;*p;++p) {
        if(*p=='"' || *p=='\\') { putchar('\\'); putchar(*p); }
        else if(*p<32) printf("\\u%04x",*p); else putchar(*p);
    }
    putchar('"');
}
static void card_json(fortune_card_t c) {
    if(c.quote==FORTUNE_NO_CARD) { printf("null"); return; }
    char text[128]; fortune_decode(c.quote,text,sizeof(text));
    printf("{\"id\":%u,\"art\":%u,\"theme\":%u,\"text\":",c.quote,c.art,
           c.quote<FORTUNE_COUNT?FORTUNE_RECORDS[c.quote].mood:0);
    json_string(text); printf(",\"citation\":"); json_string(fortune_citation(c.quote)); printf("}");
}
static void status(void) {
    write_frame(); unsigned unread[9]={0},seen=0;
    for(unsigned i=0;i<FORTUNE_COUNT;++i) {
        if(s_state.seen[i/8]&(1U<<(i%8))) ++seen;
        else { ++unread[0]; ++unread[FORTUNE_RECORDS[i].mood]; }
    }
    printf("{\"page\":%u,\"topic\":%u,\"seed\":%u,\"cycle\":%u,\"cursor\":%u,\"seen\":%u,\"remaining\":%u,",
           s_page,s_state.mood,s_state.seed,s_state.cycle,s_state.cursor,seen,fortune_remaining(&s_state));
    printf("\"topic_candidate\":%u,",s_topic_candidate);
    printf("\"mail\":{\"story\":%u,\"page\":%u,\"style\":%u,\"reading\":%s,\"active\":%s,\"selected\":%u,\"error\":%s,\"corrupt\":%s,\"count\":%u,\"title\":",
        s_mail_view.story,s_mail_view.page,s_mail_view.style,s_mail_view.reading?"true":"false",s_mail.active?"true":"false",
        s_mail.selected,s_mail_error?"true":"false",s_mail_corrupt?"true":"false",fortune_mail_pages(s_mail_view.story));
    json_string(fortune_mail_title(s_mail_view.story)); printf(",\"text\":");
    json_string(fortune_mail_text(s_mail_view.story,s_mail_view.reading?s_mail_view.page:0));
    printf(",\"bookmarks\":[");
    for (unsigned i=0;i<FORTUNE_MAIL_STORIES;++i) printf("%s%u",i?",":"",s_mail.bookmarks[i]);
    printf("]},");
    printf("\"album_index\":%u,\"album_action\":%u,\"album_confirm\":%s,\"favorite_count\":%u,",
        s_album_index,s_album_action,s_album_confirm?"true":"false",s_state.favorite_count);
    printf("\"count\":%u,\"corpus_id\":%u,\"opening\":%s,\"volume_open\":%s,\"volume\":%u,\"candidate\":%u,",
           FORTUNE_COUNT,FORTUNE_CORPUS_ID,fortune_ui_revealing()?"true":"false",s_volume_open?"true":"false",s_volume,s_volume_candidate);
    printf("\"sound_enabled\":%s,\"audio_enabled\":%s,\"audio_volume\":%u,\"sound_id\":%u,\"sound\":%u,\"frame_id\":%u,\"drawn\":%s,\"save_error\":%s,\"notice\":",
           s_sound_enabled?"true":"false",audio_enabled?"true":"false",audio_volume,sound_id,last_sound,frame_id,drawn?"true":"false",s_save_error?"true":"false");
    json_string(s_notice?s_notice:""); printf(",\"unread\":[");
    for(unsigned i=0;i<9;++i) printf("%s%u",i?",":"",unread[i]);
    printf("],\"current\":"); card_json(s_state.current); printf(",\"pinned\":"); card_json(s_state.pinned);
    printf(",\"favorites\":[");
    for(unsigned i=0;i<s_state.favorite_count;++i) { if(i) putchar(','); card_json(s_state.favorites[i]); }
    printf("]");
    printf("}\n"); fflush(stdout);
}
int main(int argc,char **argv) {
    if(argc!=5) { fprintf(stderr,"Usage: bridge state frame.bmp sound.wav seed\n"); return 2; }
    storage_path=argv[1]; frame_path=argv[2]; wave_path=argv[3]; random_seed=(uint32_t)strtoul(argv[4],NULL,10);
    lv_init(); app_main(); advance(25); status();
    char line[128];
    while(fgets(line,sizeof(line),stdin)) {
        drawn=false;
        unsigned ms,topic,seed; char button[12],event[12];
        if(sscanf(line,"tick %u",&ms)==1) advance(ms>2000?2000:ms);
        else if(sscanf(line,"key %11s %11s",button,event)==2) {
            bsp_btn_t b=!strcmp(button,"up")?BSP_BTN_UP:!strcmp(button,"down")?BSP_BTN_DOWN:BSP_BTN_OK;
            key(b,!strcmp(event,"long")?BSP_BTN_LONG:BSP_BTN_CLICK);
        } else if(sscanf(line,"topic %u",&topic)==1 && topic<FORTUNE_TOPIC_COUNT) {
            if(fortune_ui_revealing()) key(BSP_BTN_OK,BSP_BTN_CLICK);
            if(s_volume_open) key(BSP_BTN_OK,BSP_BTN_LONG);
            while(s_page!=FORTUNE_HOME) key(BSP_BTN_OK,BSP_BTN_LONG);
            key(BSP_BTN_UP,BSP_BTN_CLICK); /* Enter the explicit selector. */
            while(s_topic_candidate!=topic) key(BSP_BTN_DOWN,BSP_BTN_CLICK);
        } else if(!strncmp(line,"draw",4)) {
            if(fortune_ui_revealing()) key(BSP_BTN_OK,BSP_BTN_CLICK);
            if(s_volume_open) key(BSP_BTN_OK,BSP_BTN_LONG);
            while(album_page() || s_page==FORTUNE_MAIL) key(BSP_BTN_OK,BSP_BTN_LONG);
            key(s_page==FORTUNE_HOME||s_page==FORTUNE_TOPICS?BSP_BTN_OK:BSP_BTN_UP,BSP_BTN_CLICK);
            if(fortune_ui_revealing()) key(BSP_BTN_OK,BSP_BTN_CLICK);
        } else if(!strncmp(line,"reboot",6)) {
            fortune_ui_finish_reveal(); s_volume_open=false; s_notice=NULL; s_save_error=false;
            load(); s_page=s_mail.active?FORTUNE_MAIL:s_state.pinned.quote!=FORTUNE_NO_CARD?FORTUNE_SHOWCASE:FORTUNE_HOME;
            fortune_audio_enable(s_sound_enabled); fortune_audio_volume(s_volume); render();
        } else if(sscanf(line,"fresh %u",&seed)==1) {
            fortune_ui_finish_reveal(); fortune_defaults(&s_state,seed);
            fortune_mail_t empty; fortune_mail_defaults(&empty); s_mail_corrupt=false; (void)store_mail(&empty);
            s_mail_view=(fortune_mail_view_t){0}; s_mail_bag=0; s_mail_random=seed?seed:1;
            s_page=FORTUNE_HOME; s_volume_open=false; s_volume=80; s_sound_enabled=true; s_notice=NULL;
            fortune_audio_enable(true); fortune_audio_volume(80); save_after_change(); render();
        } else if(!strncmp(line,"quit",4)) break;
        advance(1); status();
    }
    return 0;
}
