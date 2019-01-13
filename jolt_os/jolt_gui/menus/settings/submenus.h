#ifndef JOLT_GUI_MENUS_SETTINGS_SUBMENUS_H__
#define JOLT_GUI_MENUS_SETTINGS_SUBMENUS_H__

#include "lvgl/lvgl.h"

lv_res_t menu_wifi_details_create(lv_obj_t *btn);
lv_res_t menu_screen_brightness_create();
lv_res_t menu_language_create(lv_obj_t *btn);
lv_res_t menu_factory_reset_create(lv_obj_t *btn);

#endif
