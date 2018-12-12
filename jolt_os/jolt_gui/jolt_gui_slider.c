#include "jolt_gui.h"
#include "jolt_gui_slider.h"

lv_obj_t *jolt_gui_scr_slider_get_slider(lv_obj_t *scr) {
    lv_obj_t *child = NULL;
    lv_obj_type_t obj_type;

    lv_obj_t *cont = NULL;
    lv_obj_t *slider = NULL;

	while( NULL != (child = lv_obj_get_child(scr, child)) ) {
		lv_obj_get_type(child, &obj_type);
		if( 0 == strcmp("lv_cont", obj_type.type[0]) ) {
            cont = child;
		}
	}
	while( NULL != (child = lv_obj_get_child(cont, child)) ) {
		lv_obj_get_type(child, &obj_type);
		if( 0 == strcmp("lv_slider", obj_type.type[0]) ) {
            slider = child;
		}
	}

    return slider;
}

int16_t jolt_gui_scr_slider_get_value(lv_obj_t *scr) {
    lv_obj_t *slider = jolt_gui_scr_slider_get_slider(scr);
    return lv_slider_get_value(slider);
}

void jolt_gui_scr_slider_set_value(lv_obj_t *scr, int16_t value) {
    lv_obj_t *slider = jolt_gui_scr_slider_get_slider(scr);
    lv_slider_set_value(slider, value);
}

void jolt_gui_scr_slider_set_range(lv_obj_t *scr, int16_t min, int16_t max) {
    lv_obj_t *slider = jolt_gui_scr_slider_get_slider(scr);
    lv_slider_set_range(slider, min, max);
}


lv_obj_t *jolt_gui_scr_slider_create(const char *title, lv_action_t cb) {
    lv_obj_t *parent = jolt_gui_parent_create();

    /* Create Title */
    lv_obj_t *title_label = jolt_gui_obj_title_create(parent, title);

    /* Create Container */
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, LV_HOR_RES, 
            LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H);
	lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_LEFT,
            0, CONFIG_JOLT_GUI_STATUSBAR_H);

    /* Create Slider */
    lv_obj_t *slider = lv_slider_create(cont, NULL);
    lv_slider_set_range(slider, 0, 10);
    lv_obj_set_size(slider,
            CONFIG_JOLT_GUI_LOADING_BAR_W, CONFIG_JOLT_GUI_LOADING_BAR_H);
	lv_obj_align(slider, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_slider_set_action(slider, cb);
    lv_slider_set_value(slider, 0);

    lv_group_add_obj(jolt_gui_store.group.main, slider);
    lv_group_focus_obj(slider);

    jolt_gui_scr_set_back_action(parent, jolt_gui_scr_del);

    return parent;
}
