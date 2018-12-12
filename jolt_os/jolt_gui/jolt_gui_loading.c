#include "jolt_gui.h"
#include "jolt_gui_loading.h"

/* Update the loading screen.
 * Title and Text are optional.*/
void jolt_gui_scr_loading_update(lv_obj_t *parent,
        const char *title, const char *text,
        uint8_t percentage) {
    // get the bar children
    lv_obj_t *child = NULL;
    lv_obj_type_t obj_type;
    lv_obj_t *cont = NULL;
    lv_obj_t *bar = NULL;
    lv_obj_t *loading_label = NULL;
    lv_obj_t *title_label = NULL;

	while( NULL != (child = lv_obj_get_child(parent, child)) ) {
		lv_obj_get_type(child, &obj_type);
		if( 0 == strcmp("lv_cont", obj_type.type[0]) ) {
            cont = child;
		}
        else if( 0 == strcmp("lv_label", obj_type.type[0]) ) {
            // this might be the title
            title_label = child;
        }
	}
	while( NULL != (child = lv_obj_get_child(cont, child)) ) {
		lv_obj_get_type(child, &obj_type);
		if( 0 == strcmp("lv_bar", obj_type.type[0]) ) {
            bar = child;
		}
        else if( 0 == strcmp("lv_label", obj_type.type[0]) ) {
            // this might be the title
            loading_label = child;
        }
	}

    if( percentage > 100 ) {
        percentage = 100;
    }
	lv_bar_set_value_anim(bar, percentage, CONFIG_JOLT_GUI_LOADING_BAR_ANIM_MS);
    if( text ) {
        lv_label_set_text(loading_label, text);
    }
    if( title ) {
        lv_label_set_text(title_label, title);
    }
}

/* Creates and returns a loading screen.
 *
 * Defaults the back and enter action to NULL */
lv_obj_t *jolt_gui_scr_loading_create(const char *title) {
    lv_obj_t *parent = jolt_gui_parent_create();

    /* Create Title */
    lv_obj_t *title_label = jolt_gui_obj_title_create(parent, title);

    /* Create Container */
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, LV_HOR_RES, 
            LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H);
	lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_LEFT,
            0, CONFIG_JOLT_GUI_STATUSBAR_H);

    /* Create Loading Bar */
    lv_obj_t *bar = lv_bar_create(cont, NULL);
	lv_obj_set_size(bar, 
            CONFIG_JOLT_GUI_LOADING_BAR_W, CONFIG_JOLT_GUI_LOADING_BAR_H);
	lv_obj_align(bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
	lv_bar_set_value(bar, 1);

    lv_group_add_obj(jolt_gui_store.group.main, cont);
    lv_group_focus_obj(cont);

    /* Create Loading Label */
    lv_obj_t *label = lv_label_create(cont, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label, "Initializing");
    lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
    lv_style_t *label_style = lv_obj_get_style(label);
    lv_obj_set_size(label, CONFIG_JOLT_GUI_LOADING_TEXT_W,
            label_style->text.font->h_px);
	lv_obj_align(label, bar, LV_ALIGN_OUT_TOP_MID, 0, -10);

    jolt_gui_scr_set_back_action(parent, NULL);
    jolt_gui_scr_set_enter_action(parent, NULL);

    return parent;
}


