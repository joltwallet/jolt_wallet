#include "jolt_gui_menu.h"
#include "jolt_gui.h"

/* Menu Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--LIST
 */
static const char TAG[] = "scr_menu";

/* Creates a standard Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_create(const char *title) {
    JOLT_GUI_SCR_PREAMBLE( title );

    /* Create List */
    // todo, get these values from parent
    lv_obj_t *menu = lv_list_create(cont_body, NULL);
    JOLT_GUI_OBJ_CHECK(menu);
    lv_obj_set_free_num(menu, JOLT_GUI_OBJ_ID_LIST);
    lv_obj_set_size(menu,
            lv_obj_get_width(cont_body), lv_obj_get_height(cont_body));
    lv_list_set_sb_mode(menu, LV_SB_MODE_AUTO);
    lv_obj_align(menu, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    jolt_gui_group_add( menu );

    jolt_gui_scr_set_enter_action(parent, jolt_gui_send_enter_main);
    jolt_gui_scr_set_back_action(parent, jolt_gui_scr_del);

exit:
    return parent;
}

/* Gets the list object of a menu screen */
lv_obj_t *jolt_gui_scr_menu_get_list(lv_obj_t *parent) {
    lv_obj_t *menu = NULL;
    {
        lv_obj_t *cont_body = NULL;
        cont_body  = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        menu       = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_LIST);
    }

exit:
    return menu;
}

/* Adds an item to a Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_add(lv_obj_t *par, const void *img_src,
        const char *txt, lv_action_t rel_action) {
    lv_obj_t *btn = NULL;
    lv_obj_t *list = jolt_gui_scr_menu_get_list( par );
    if( NULL == list ) {
        return NULL;
    }
    btn = lv_list_add(list, img_src, txt, rel_action);
    if( NULL == btn ){
        goto exit;
    }

exit:
    return btn;
}
