#include "jolt_gui.h"
#include "jolt_gui_loading.h"

/* Loading and Preloading Screen Structures:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--BAR_LOADING or PRELOADING
 *       +--LABEL_0 (loading label)
 */

static const char TAG[] = "scr_loading";

/* Update the loading screen.
 * Title and Text are optional.*/
void jolt_gui_scr_loading_update(lv_obj_t *parent,
        const char *title, const char *text,
        uint8_t percentage) {

    /* Find Objects */
    lv_obj_t *label_title   = NULL;
    {
        lv_obj_t *cont_title = NULL;
        cont_title  = JOLT_GUI_FIND_AND_CHECK(parent,     JOLT_GUI_OBJ_ID_CONT_TITLE);
        label_title = JOLT_GUI_FIND_AND_CHECK(cont_title, JOLT_GUI_OBJ_ID_LABEL_0);
    }

    lv_obj_t *bar_loading   = NULL;
    lv_obj_t *label_loading = NULL;
    {
        lv_obj_t *cont_body = NULL;
        cont_body     = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        bar_loading   = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_BAR_LOADING);
        label_loading = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_LABEL_0);
    }

    if( percentage > 100 ) {
        percentage = 100;
    }
	lv_bar_set_value_anim(bar_loading, percentage, CONFIG_JOLT_GUI_LOADING_BAR_ANIM_MS);
    if( text ) {
        lv_label_set_text(label_loading, text);
    }
    if( title ) {
        lv_label_set_text(label_title, title);
    }
exit:
    return;
}

/* Creates and returns a loading screen.
 * Defaults the back and enter action to NULL.
 * */
lv_obj_t *jolt_gui_scr_loading_create(const char *title) {
    JOLT_GUI_SCR_PREAMBLE( title );

    /* Create Loading Bar */
    lv_obj_t *bar = lv_bar_create(cont_body, NULL);
    JOLT_GUI_OBJ_CHECK( bar );
    lv_obj_set_free_num(bar, JOLT_GUI_OBJ_ID_BAR_LOADING);
	lv_obj_set_size(bar, 
            CONFIG_JOLT_GUI_LOADING_BAR_W, CONFIG_JOLT_GUI_LOADING_BAR_H);
	lv_obj_align(bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
	lv_bar_set_value(bar, 1);

    /* Create Loading Label */
    lv_obj_t *label = lv_label_create(cont_body, NULL);
    JOLT_GUI_OBJ_CHECK( label );
    lv_obj_set_free_num(label, JOLT_GUI_OBJ_ID_LABEL_0);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(label, "Initializing");
    lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);

    /* Set Label Size and Position */
    lv_style_t *label_style = lv_obj_get_style(label);
    lv_obj_set_size(label, CONFIG_JOLT_GUI_LOADING_TEXT_W,
            label_style->text.font->h_px);
	lv_obj_align(label, bar, LV_ALIGN_OUT_TOP_MID, 0, -10);

    jolt_gui_scr_set_back_action(parent, NULL);
    jolt_gui_scr_set_enter_action(parent, NULL);

exit:
    return parent;
}

/* Creates and returns a preloading screen.
 * A preloading screen has no progression and is used for tasks of unknown 
 * lenth and/or progression.
 *
 * Defaults the back and enter action to NULL.
  * */
lv_obj_t *jolt_gui_scr_preloading_create(const char *title, const char *text) {
    JOLT_GUI_SCR_PREAMBLE( title );

    /* Create PreLoading Object */
    lv_obj_t *preload = lv_preload_create(cont_body, NULL);
    JOLT_GUI_OBJ_CHECK( preload );
    lv_obj_set_free_num(preload, JOLT_GUI_OBJ_ID_PRELOADING);
    lv_obj_set_size(preload, 30, 30);
	lv_obj_align(preload, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    /* Create Loading Label */
    lv_obj_t *label = lv_label_create(cont_body, NULL);
    JOLT_GUI_OBJ_CHECK( label );
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
	lv_obj_align(label, preload, LV_ALIGN_OUT_TOP_MID, 0, -6);

    jolt_gui_scr_set_back_action(parent, NULL);
    jolt_gui_scr_set_enter_action(parent, NULL);

exit:
    return parent;
}


