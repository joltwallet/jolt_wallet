#include "jolt_gui.h"
#include "jolt_gui_slider.h"
#include "esp_log.h"

/* Slider Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--SLIDER
 *            +--LABEL_0
 */

static const char TAG[] = "scr_slider";

lv_obj_t *jolt_gui_scr_slider_get_slider(lv_obj_t *parent) {
    lv_obj_t *slider = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body  = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        slider     = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_SLIDER);
    }
    return slider;
}

int16_t jolt_gui_scr_slider_get_value(lv_obj_t *scr) {
    uint16_t val = 0;
    JOLT_GUI_CTX{
        lv_obj_t *slider = BREAK_IF_NULL(jolt_gui_scr_slider_get_slider(scr));
        val = lv_slider_get_value(slider);
    }
    return val;
}

void jolt_gui_scr_slider_set_value(lv_obj_t *scr, int16_t value) {
    JOLT_GUI_CTX{
        lv_obj_t *slider = jolt_gui_scr_slider_get_slider(scr);
        lv_slider_set_value(slider, value);
    }
}

void jolt_gui_scr_slider_set_range(lv_obj_t *scr, int16_t min, int16_t max) {
    JOLT_GUI_CTX{
        lv_obj_t *slider = jolt_gui_scr_slider_get_slider(scr);
        lv_slider_set_range(slider, min, max);
    }
}

void jolt_gui_scr_slider_set_label(lv_obj_t *scr, const char *text){
    JOLT_GUI_CTX{
        lv_obj_t *slider = jolt_gui_scr_slider_get_slider(scr);
        lv_obj_t *label = NULL;
        label = JOLT_GUI_FIND_AND_CHECK(slider, JOLT_GUI_OBJ_ID_LABEL_0);
        lv_label_set_text(label, text);
    }
}


/* Creates a screen with the given title, creates a slider, a label above the 
 * slider, and will activate the callback everytime the slider value changes.
 *
 * Back button is set to screen delete by default. */
lv_obj_t *jolt_gui_scr_slider_create(const char *title, const char *text, lv_action_t cb) {
    JOLT_GUI_SCR_CTX( title ) {
        /* Create Slider */
        lv_obj_t *slider = lv_slider_create(cont_body, NULL);
        lv_obj_set_free_num(slider, JOLT_GUI_OBJ_ID_SLIDER);
        lv_slider_set_range(slider, 0, 10); // Default Slider Range
        lv_obj_set_size(slider,
                CONFIG_JOLT_GUI_LOADING_BAR_W, CONFIG_JOLT_GUI_LOADING_BAR_H);
        lv_obj_align(slider, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        lv_slider_set_action(slider, cb);
        lv_slider_set_value(slider, 0); // Default initial value

        jolt_gui_group_add( slider );

        /* Create Label */
        lv_obj_t *label = BREAK_IF_NULL(lv_label_create(slider, NULL));
        lv_obj_set_free_num(label, JOLT_GUI_OBJ_ID_LABEL_0);
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
        if( NULL == text ){
            lv_label_set_text(label, "");
        }
        else {
            lv_label_set_text(label, text);
        }
        lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
        lv_style_t *label_style = lv_obj_get_style(label);
        lv_obj_set_size(label, CONFIG_JOLT_GUI_LOADING_TEXT_W,
                label_style->text.font->h_px);
        lv_obj_align(label, NULL, LV_ALIGN_OUT_TOP_MID, 0, -6);


        BREAK_IF_NULL(jolt_gui_scr_set_enter_action(parent, NULL));
        BREAK_IF_NULL(jolt_gui_scr_set_back_action(parent, jolt_gui_scr_del));
    }
    return parent;
}
