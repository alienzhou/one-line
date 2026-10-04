#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#define portMAX_DELAY UINT32_MAX
#define pdMS_TO_TICKS(ms) (ms)
typedef int BaseType_t;
typedef uint32_t TickType_t;
typedef void *TaskHandle_t;
typedef struct test_channel *QueueHandle_t;
typedef struct test_channel *SemaphoreHandle_t;
const char *esp_err_to_name(esp_err_t error);
void audio_test_log(const char *tag,const char *format,...);
#define ESP_LOGW(...) audio_test_log(__VA_ARGS__)
esp_err_t bsp_audio_init(void);
esp_err_t bsp_audio_set_format(uint32_t hz,uint8_t bits,uint8_t channels);
esp_err_t bsp_audio_wake(void);
esp_err_t bsp_audio_sleep(void);
esp_err_t bsp_audio_write(const void *pcm,size_t bytes);
void bsp_audio_set_volume(uint8_t volume);
QueueHandle_t xQueueCreate(unsigned length,size_t size);
void vQueueDelete(QueueHandle_t queue);
BaseType_t xQueueOverwrite(QueueHandle_t queue,const void *item);
BaseType_t xQueueReceive(QueueHandle_t queue,void *item,TickType_t wait);
SemaphoreHandle_t xSemaphoreCreateBinary(void);
void vSemaphoreDelete(SemaphoreHandle_t sem);
BaseType_t xSemaphoreGive(SemaphoreHandle_t sem);
BaseType_t xSemaphoreTake(SemaphoreHandle_t sem,TickType_t wait);
BaseType_t xTaskCreate(void (*entry)(void *),const char *name,unsigned stack,void *argument,
                       unsigned priority,TaskHandle_t *handle);
TickType_t xTaskGetTickCount(void);
