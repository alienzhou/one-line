#include "fortune_audio.h"
#include "bsp_audio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdatomic.h>
#include <string.h>

#define CHUNK 160U
#define IDLE_MS 2500U
#define QUIET_MS 1000U
#define VOLUME 60U
typedef struct { fortune_sound_t sound; unsigned variant; uint32_t quiet_ticket; } command_t;
static const char *TAG="fortune_audio";
static QueueHandle_t s_mailbox;
static SemaphoreHandle_t s_quiet;
static atomic_bool s_enabled, s_paused, s_failed;
static atomic_uint s_ack, s_requested;
static uint32_t s_ticket;

static void audio_worker(void *argument) {
    (void)argument;
    bool initialized=false, opened=false, sleeping=false;
    command_t current={0}, next;
    uint32_t offset=0;
    int16_t pcm[CHUNK], last=0;
    for(;;) {
        bool playing=offset<fortune_sound_samples(current.sound);
        TickType_t wait=playing?0:opened?pdMS_TO_TICKS(IDLE_MS):portMAX_DELAY;
        if(xQueueReceive(s_mailbox,&next,wait)==pdTRUE) {
            /* A racing UI event cannot overwrite the storage stop handshake. */
            uint32_t requested=atomic_load(&s_requested);
            if(requested!=atomic_load(&s_ack)) next=(command_t){.quiet_ticket=requested};
            /* Smoothly release a cut-off note before switching or muting. */
            if(opened && last) {
                for(unsigned i=0;i<CHUNK;++i) pcm[i]=(int16_t)((int32_t)last*(int32_t)(CHUNK-1-i)/(int32_t)CHUNK);
                if(bsp_audio_write(pcm,sizeof(pcm))!=ESP_OK) atomic_store(&s_failed,true);
            }
            current=next; offset=0; last=0;
            if(current.sound==FORTUNE_SOUND_NONE || !atomic_load(&s_enabled) || atomic_load(&s_paused)) {
                current.sound=FORTUNE_SOUND_NONE;
                if(opened) {
                    memset(pcm,0,sizeof(pcm));
                    /* 100 ms of zero PCM drains the BSP's <=90 ms DMA queue. */
                    for(unsigned i=0;i<10;++i) if(bsp_audio_write(pcm,sizeof(pcm))!=ESP_OK) break;
                    bsp_audio_set_volume(0);
                    if(bsp_audio_sleep()!=ESP_OK) atomic_store(&s_failed,true);
                    opened=false; sleeping=true;
                }
                if(next.quiet_ticket) {
                    atomic_store(&s_ack,next.quiet_ticket);
                    xSemaphoreGive(s_quiet);
                }
                continue;
            }
            esp_err_t result=ESP_OK;
            if(!initialized) {
                result=bsp_audio_init(); initialized=result==ESP_OK;
            }
            if(result==ESP_OK && !opened) {
                bsp_audio_set_volume(0);
                result=sleeping?bsp_audio_wake():bsp_audio_set_format(FORTUNE_SOUND_HZ,16,1);
                if(result==ESP_OK) {
                    opened=true; sleeping=false;
                    memset(pcm,0,sizeof(pcm));
                    result=bsp_audio_write(pcm,sizeof(pcm));
                    bsp_audio_set_volume(VOLUME);
                }
            }
            if(result!=ESP_OK) {
                ESP_LOGW(TAG,"Sound unavailable: %s",esp_err_to_name(result));
                atomic_store(&s_failed,true); current.sound=FORTUNE_SOUND_NONE;
                if(initialized) { (void)bsp_audio_sleep(); sleeping=true; opened=false; }
                continue;
            }
            atomic_store(&s_failed,false);
        } else if(!playing && opened) {
            bsp_audio_set_volume(0);
            if(bsp_audio_sleep()!=ESP_OK) atomic_store(&s_failed,true);
            opened=false; sleeping=true;
        }
        size_t n=fortune_sound_render(current.sound,current.variant,offset,pcm,CHUNK);
        if(!n) continue;
        if(bsp_audio_write(pcm,n*sizeof(*pcm))!=ESP_OK) {
            ESP_LOGW(TAG,"PCM write failed; keeping the card usable");
            atomic_store(&s_failed,true); current.sound=FORTUNE_SOUND_NONE;
            (void)bsp_audio_sleep(); opened=false; sleeping=true; last=0;
        } else { offset+=(uint32_t)n; last=pcm[n-1]; }
    }
}

bool fortune_audio_start(bool enabled) {
    if(s_mailbox) return true;
    atomic_store(&s_enabled,enabled);
    s_mailbox=xQueueCreate(1,sizeof(command_t)); s_quiet=xSemaphoreCreateBinary();
    if(!s_mailbox || !s_quiet || xTaskCreate(audio_worker,"fortune_sound",4096,NULL,5,NULL)!=pdPASS) {
        if(s_mailbox) vQueueDelete(s_mailbox);
        if(s_quiet) vSemaphoreDelete(s_quiet);
        s_mailbox=NULL; s_quiet=NULL; atomic_store(&s_failed,true); return false;
    }
    return true;
}
void fortune_audio_play(fortune_sound_t sound,unsigned variant) {
    if(!s_mailbox || !atomic_load(&s_enabled) || atomic_load(&s_paused) ||
       sound<=FORTUNE_SOUND_NONE || sound>=FORTUNE_SOUND_COUNT) return;
    command_t command={sound,variant%FORTUNE_SOUND_VARIANTS,0};
    xQueueOverwrite(s_mailbox,&command);
}
void fortune_audio_enable(bool enabled) {
    atomic_store(&s_enabled,enabled);
    if(!enabled && s_mailbox) { command_t stop={0}; xQueueOverwrite(s_mailbox,&stop); }
}
bool fortune_audio_failed(void) { return atomic_load(&s_failed); }
bool fortune_audio_quiet(void) {
    atomic_store(&s_paused,true);
    if(!s_mailbox) return true;
    if(++s_ticket==0) ++s_ticket;
    atomic_store(&s_requested,s_ticket);
    command_t command={.quiet_ticket=s_ticket};
    xQueueOverwrite(s_mailbox,&command);
    TickType_t start=xTaskGetTickCount(), timeout=pdMS_TO_TICKS(QUIET_MS);
    while(atomic_load(&s_ack)!=s_ticket) {
        TickType_t elapsed=xTaskGetTickCount()-start;
        if(elapsed>=timeout || xSemaphoreTake(s_quiet,timeout-elapsed)!=pdTRUE) return false;
    }
    return true;
}
void fortune_audio_resume(void) { atomic_store(&s_paused,false); }
