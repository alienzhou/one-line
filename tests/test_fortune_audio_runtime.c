#define _POSIX_C_SOURCE 200809L
#include "fortune_audio_stubs/platform.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../main/fortune_audio.c"

/* Execute the actual worker against a blocking RTOS/BSP boundary. Time runs
 * ten times faster; production synchronization and PCM are not replaced. */
struct test_channel { pthread_mutex_t mutex; pthread_cond_t cond; bool full; size_t size; char value[32]; };
static pthread_t thread;
static void (*task_entry)(void *);
static void *task_argument;
static atomic_bool stopping,hold_write,device_open;
static atomic_uint writes,sleeps,wakes,nonzero_writes,fail_stage;
static unsigned allocation_failure;
static int16_t captured[100000]; static size_t captured_count;
static void delay_us(long us) { struct timespec t={us/1000000,(us%1000000)*1000}; nanosleep(&t,NULL); }
static struct timespec deadline(TickType_t ticks) {
    struct timespec t; clock_gettime(CLOCK_REALTIME,&t);
    uint64_t ns=(uint64_t)t.tv_nsec+(uint64_t)ticks*100000U;
    t.tv_sec+=(time_t)(ns/1000000000U); t.tv_nsec=(long)(ns%1000000000U); return t;
}
TickType_t xTaskGetTickCount(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return (TickType_t)((uint64_t)t.tv_sec*10000U+t.tv_nsec/100000U); }
QueueHandle_t xQueueCreate(unsigned length,size_t size) {
    assert(length==1 && size<=32); if(allocation_failure==1) return NULL;
    QueueHandle_t q=calloc(1,sizeof(*q)); assert(q); q->size=size;
    pthread_mutex_init(&q->mutex,NULL); pthread_cond_init(&q->cond,NULL); return q;
}
void vQueueDelete(QueueHandle_t q) { pthread_cond_destroy(&q->cond); pthread_mutex_destroy(&q->mutex); free(q); }
BaseType_t xQueueOverwrite(QueueHandle_t q,const void *item) {
    pthread_mutex_lock(&q->mutex); memcpy(q->value,item,q->size); q->full=true;
    pthread_cond_signal(&q->cond); pthread_mutex_unlock(&q->mutex); return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t q,void *item,TickType_t ticks) {
    pthread_mutex_lock(&q->mutex); struct timespec until=deadline(ticks);
    while(!q->full && ticks && !atomic_load(&stopping)) {
        int r=ticks==portMAX_DELAY?pthread_cond_wait(&q->cond,&q->mutex):pthread_cond_timedwait(&q->cond,&q->mutex,&until);
        if(r==ETIMEDOUT) break;
        assert(r==0);
    }
    if(atomic_load(&stopping)) { pthread_mutex_unlock(&q->mutex); pthread_exit(NULL); }
    bool full=q->full;
    if(full) { memcpy(item,q->value,q->size); q->full=false; }
    pthread_mutex_unlock(&q->mutex); return full?pdTRUE:pdFALSE;
}
SemaphoreHandle_t xSemaphoreCreateBinary(void) { return allocation_failure==2?NULL:xQueueCreate(1,1); }
void vSemaphoreDelete(SemaphoreHandle_t q) { vQueueDelete(q); }
BaseType_t xSemaphoreGive(SemaphoreHandle_t q) { const char value=1; return xQueueOverwrite(q,&value); }
BaseType_t xSemaphoreTake(SemaphoreHandle_t q,TickType_t ticks) { char value; return xQueueReceive(q,&value,ticks); }
static void *task_wrapper(void *unused) { (void)unused; task_entry(task_argument); return NULL; }
BaseType_t xTaskCreate(void (*entry)(void *),const char *name,unsigned stack,void *argument,unsigned priority,TaskHandle_t *handle) {
    (void)name; (void)handle; assert(stack==4096 && priority==5);
    if(allocation_failure==3) return pdFALSE;
    task_entry=entry; task_argument=argument; assert(pthread_create(&thread,NULL,task_wrapper,NULL)==0); return pdPASS;
}
static bool fail(unsigned stage) { unsigned expected=stage; return atomic_compare_exchange_strong(&fail_stage,&expected,0); }
static void owner(void) { assert(pthread_equal(pthread_self(),thread)); }
const char *esp_err_to_name(esp_err_t error) { (void)error; return "injected"; }
void audio_test_log(const char *tag,const char *format,...) { (void)tag; (void)format; }
esp_err_t bsp_audio_init(void) { owner(); return fail(1)?ESP_FAIL:ESP_OK; }
esp_err_t bsp_audio_set_format(uint32_t hz,uint8_t bits,uint8_t channels) {
    owner(); assert(hz==16000 && bits==16 && channels==1);
    if(fail(2)) return ESP_FAIL;
    atomic_store(&device_open,true); return ESP_OK;
}
esp_err_t bsp_audio_wake(void) { owner(); atomic_fetch_add(&wakes,1); if(fail(3)) return ESP_FAIL; atomic_store(&device_open,true); return ESP_OK; }
esp_err_t bsp_audio_sleep(void) { owner(); atomic_fetch_add(&sleeps,1); atomic_store(&device_open,false); return ESP_OK; }
void bsp_audio_set_volume(uint8_t volume) { owner(); assert(volume==0 || volume==60); }
esp_err_t bsp_audio_write(const void *pcm,size_t bytes) {
    owner(); assert(atomic_load(&device_open)); assert(bytes>0 && bytes<=320 && bytes%2==0);
    atomic_fetch_add(&writes,1);
    while(atomic_load(&hold_write)) delay_us(1000);
    if(fail(4)) return ESP_FAIL;
    const int16_t *samples=pcm; bool nonzero=false;
    assert(captured_count+bytes/2<=100000);
    for(size_t i=0;i<bytes/2;++i) { if(samples[i]) nonzero=true; captured[captured_count++]=samples[i]; }
    if(nonzero) atomic_fetch_add(&nonzero_writes,1);
    delay_us(1000); return ESP_OK;
}
static void until_at_least(atomic_uint *counter,unsigned target) {
    for(unsigned i=0;i<2000 && atomic_load(counter)<target;++i) delay_us(1000);
    assert(atomic_load(counter)>=target);
}
static void until_failed(void) { for(unsigned i=0;i<2000 && !fortune_audio_failed();++i) delay_us(1000); assert(fortune_audio_failed()); }
static void stop_fixture(void) {
    atomic_store(&stopping,true); command_t stop={0}; xQueueOverwrite(s_mailbox,&stop);
    assert(pthread_join(thread,NULL)==0); vQueueDelete(s_mailbox); vSemaphoreDelete(s_quiet);
    s_mailbox=NULL; s_quiet=NULL; atomic_store(&stopping,false);
    atomic_store(&s_failed,false); atomic_store(&s_paused,false);
    atomic_store(&s_ack,0); atomic_store(&s_requested,0); s_ticket=0; captured_count=0;
}
int main(void) {
    for(allocation_failure=1;allocation_failure<=3;++allocation_failure) {
        assert(!fortune_audio_start(true)); assert(!s_mailbox && !s_quiet);
    }
    allocation_failure=0; assert(fortune_audio_start(false)); atomic_store(&s_failed,false);
    fortune_audio_play(FORTUNE_SOUND_OPEN,0); delay_us(10000); assert(atomic_load(&writes)==0);
    fortune_audio_enable(true); atomic_store(&hold_write,true);
    fortune_audio_play(FORTUNE_SOUND_OPEN,0); until_at_least(&writes,1);
    for(unsigned i=0;i<30;++i) fortune_audio_play(FORTUNE_SOUND_REVEAL,i);
    fortune_audio_play(FORTUNE_SOUND_SKIN,3); atomic_store(&hold_write,false);
    until_at_least(&nonzero_writes,20); delay_us(50000);
    assert(fortune_audio_quiet()); assert(!atomic_load(&device_open));
    int16_t expected[4480]; size_t n=fortune_sound_render(FORTUNE_SOUND_SKIN,3,0,expected,4480); bool found=false;
    for(size_t i=0;i+n<=captured_count;++i) if(!memcmp(captured+i,expected,n*2)) { found=true; break; }
    assert(found); /* Latest skin cue replaces the thirty queued reveals. */
    unsigned before=atomic_load(&writes); fortune_audio_play(FORTUNE_SOUND_KEEP,0); delay_us(10000);
    assert(atomic_load(&writes)==before); fortune_audio_resume();
    fortune_audio_enable(false); assert(fortune_audio_quiet()); fortune_audio_resume();
    before=atomic_load(&writes); fortune_audio_play(FORTUNE_SOUND_REVEAL,0); delay_us(10000); assert(atomic_load(&writes)==before);
    fortune_audio_enable(true); fortune_audio_play(FORTUNE_SOUND_SKIN,0);
    until_at_least(&writes,before+3); unsigned slept=atomic_load(&sleeps); until_at_least(&sleeps,slept+1);
    assert(!atomic_load(&device_open)); unsigned woke=atomic_load(&wakes);
    fortune_audio_play(FORTUNE_SOUND_OPEN,0); until_at_least(&wakes,woke+1); assert(fortune_audio_quiet()); fortune_audio_resume();
    /* A stuck PCM call must time out instead of falsely allowing a Flash save. */
    before=atomic_load(&writes); atomic_store(&hold_write,true); fortune_audio_play(FORTUNE_SOUND_OPEN,0);
    until_at_least(&writes,before+1); assert(!fortune_audio_quiet());
    atomic_store(&hold_write,false); assert(fortune_audio_quiet()); fortune_audio_resume(); stop_fixture();
    for(unsigned stage=1;stage<=4;++stage) {
        assert(fortune_audio_start(true)); atomic_store(&fail_stage,stage);
        if(stage==3) { fortune_audio_play(FORTUNE_SOUND_SKIN,0); delay_us(50000); assert(fortune_audio_quiet()); fortune_audio_resume(); }
        fortune_audio_play(FORTUNE_SOUND_OPEN,0); until_failed();
        before=atomic_load(&nonzero_writes); fortune_audio_play(FORTUNE_SOUND_SKIN,0);
        until_at_least(&nonzero_writes,before+1); assert(!fortune_audio_failed());
        assert(fortune_audio_quiet()); fortune_audio_resume(); stop_fixture();
    }
    puts("Fortune audio worker: PASS (ownership, mute, latest event, drain-before-save, timeout, idle/wake, init/format/wake/PCM failures)");
    return 0;
}
