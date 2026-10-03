idf_component_register(
    SRCS "main.c" "fortune_model.c" "fortune_data.c" "fortune_ui.c" "fortune_pixels.c"
         "../assets/fonts/fortune_font_12.c" "../assets/fonts/fortune_font_20.c"
    INCLUDE_DIRS "." "../assets/fonts"
    REQUIRES bsp esp_hw_support esp_timer nvs_flash
)
