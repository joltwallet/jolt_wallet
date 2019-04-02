#ifndef __JOLT_GUI_TEST_SCREENS_H__
#define __JOLT_GUI_TEST_SCREENS_H__

#if JOLT_GUI_TEST_MENU
#include "lvgl/lvgl.h"

lv_res_t jolt_gui_test_json_create(lv_obj_t *btn);
lv_res_t jolt_gui_test_qrcode_create(lv_obj_t *btn);
lv_res_t jolt_gui_test_loading_create(lv_obj_t *btn);
lv_res_t jolt_gui_test_number_create(lv_obj_t *btn);
lv_res_t jolt_gui_test_battery_create(lv_obj_t *btn);
lv_res_t jolt_gui_test_alphabet_create(lv_obj_t * list_btn);
lv_res_t jolt_gui_test_https_create( lv_obj_t *btn );


#endif // JOLT_GUI_TEST_MENU
#endif // Guard
