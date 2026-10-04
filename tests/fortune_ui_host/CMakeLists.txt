cmake_minimum_required(VERSION 3.16)
project(fortune_ui_host C)
set(CMAKE_C_STANDARD 11)
if(NOT EXISTS "${LVGL_PATH}/lvgl.h")
    message(FATAL_ERROR "LVGL_PATH must point to the managed LVGL 9.5.0 source")
endif()
file(GLOB_RECURSE LVGL_SOURCES "${LVGL_PATH}/src/*.c")
add_library(fortune_lvgl STATIC ${LVGL_SOURCES})
target_include_directories(fortune_lvgl PUBLIC "${LVGL_PATH}")
target_compile_definitions(fortune_lvgl PUBLIC LV_CONF_PATH="${CMAKE_CURRENT_LIST_DIR}/lv_conf.h")
add_executable(test_fortune_ui ../test_fortune_ui.c
    ../../main/fortune_ui.c ../../main/fortune_pixels.c ../../main/fortune_model.c ../../main/fortune_data.c ../../main/fortune_legacy_data.c
    ../../components/bsp/src/bsp_display_rounding.c
    ../../assets/fonts/fortune_font_12.c ../../assets/fonts/fortune_font_20.c)
target_include_directories(test_fortune_ui PRIVATE ../../main ../../assets/fonts
    ../bsp_stubs ../../components/bsp/include ../../components/bsp/src)
target_link_libraries(test_fortune_ui PRIVATE fortune_lvgl)
if(UNIX AND NOT APPLE)
    target_link_libraries(test_fortune_ui PRIVATE m)
endif()
target_compile_options(test_fortune_ui PRIVATE -Wall -Wextra -Werror -UNDEBUG)
