#ifndef JOLT_GUI_SCROLL_H__
#define JOLT_GUI_SCROLL_H__

#include "jolt_gui_qr.h"

lv_obj_t *jolt_gui_scr_scroll_create(const char *title);
lv_obj_t *jolt_gui_scr_scroll_add_text(lv_obj_t *scr, const char *text);
lv_obj_t *jolt_gui_scr_scroll_add_monospace_text(lv_obj_t *scr, const char *text);

lv_obj_t *jolt_gui_scr_scroll_get_page(lv_obj_t *scr);

#endif
