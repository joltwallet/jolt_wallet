#ifndef __JOLT_GUI_SLIDER_H__
#define __JOLT_GUI_SLIDER_H__

#include "jolt_gui.h"
#include "lvgl/lvgl.h"

lv_obj_t *jolt_gui_scr_slider_get_slider(lv_obj_t *scr);

lv_obj_t *jolt_gui_scr_slider_create(const char *title, lv_action_t cb);

void jolt_gui_scr_slider_set_value(lv_obj_t *scr, int16_t value);

int16_t jolt_gui_scr_slider_get_value(lv_obj_t *scr);

void jolt_gui_scr_slider_set_range(lv_obj_t *scr, int16_t min, int16_t max);
#endif
