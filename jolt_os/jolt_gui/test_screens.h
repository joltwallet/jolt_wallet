#ifndef JOLT_GUI_TEST_SCREENS_H__
#define JOLT_GUI_TEST_SCREENS_H__

#include "sdkconfig.h"

#include "jolt_gui/jolt_gui.h"

void jolt_gui_test_json_create(jolt_gui_obj_t *btn, jolt_gui_event_t event);
void jolt_gui_test_qrcode_create(jolt_gui_obj_t *btn, jolt_gui_event_t event);
void jolt_gui_test_preloading_create(jolt_gui_obj_t *btn, jolt_gui_event_t event);
void jolt_gui_test_loading_create(jolt_gui_obj_t *btn, jolt_gui_event_t event);
void jolt_gui_test_autoloading_create(jolt_gui_obj_t *btn, jolt_gui_event_t event);
void jolt_gui_test_number_create(jolt_gui_obj_t *btn, jolt_gui_event_t event);
void jolt_gui_test_battery_create(jolt_gui_obj_t *btn, jolt_gui_event_t event);
void jolt_gui_test_alphabet_create(jolt_gui_obj_t * list_btn, jolt_gui_event_t event);
void jolt_gui_test_https_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );
void jolt_gui_test_bignum_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

#endif // Guard
