#pragma once
#include "platform.h"
typedef unsigned nvs_handle_t;
#define NVS_READWRITE 1
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_NVS_NOT_FOUND 0x1102
#define MALLOC_CAP_8BIT 1
#define ESP_LOGE(...) audio_test_log(__VA_ARGS__)
#define ESP_LOGI(...) audio_test_log(__VA_ARGS__)
typedef enum { BSP_BTN_UP,BSP_BTN_DOWN,BSP_BTN_OK } bsp_btn_t;
typedef enum { BSP_BTN_PRESS,BSP_BTN_CLICK,BSP_BTN_DOUBLE,BSP_BTN_LONG } bsp_btn_ev_t;
esp_err_t bsp_button_init(void (*callback)(bsp_btn_t,bsp_btn_ev_t,void *),void *user);
esp_err_t bsp_battery_init(void);
int bsp_battery_soc(void);
esp_err_t bsp_display_init(void);
bool bsp_lvgl_init(void);
bool bsp_lvgl_lock(int timeout);
void bsp_lvgl_unlock(void);
void bsp_display_backlight(uint8_t level);
uint32_t esp_random(void);
int64_t esp_timer_get_time(void);
size_t esp_get_free_heap_size(void);
size_t heap_caps_get_largest_free_block(unsigned capability);
esp_err_t nvs_flash_init(void);
esp_err_t nvs_open(const char *name,int mode,nvs_handle_t *handle);
esp_err_t nvs_set_blob(nvs_handle_t handle,const char *key,const void *data,size_t size);
esp_err_t nvs_set_u8(nvs_handle_t handle,const char *key,uint8_t value);
esp_err_t nvs_get_blob(nvs_handle_t handle,const char *key,void *data,size_t *size);
esp_err_t nvs_get_u8(nvs_handle_t handle,const char *key,uint8_t *value);
esp_err_t nvs_commit(nvs_handle_t handle);
BaseType_t xQueueSend(QueueHandle_t queue,const void *item,TickType_t ticks);
