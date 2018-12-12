#include "jolt_gui_menu.h"
#include "jolt_gui.h"

/* Creates a standard Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_create(const char *title) {
    lv_obj_t *parent = jolt_gui_parent_create();

    /* Create List */
    lv_obj_t *menu = lv_list_create(parent, NULL);
    lv_obj_set_size(menu, LV_HOR_RES, LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H);
    lv_list_set_sb_mode(menu, LV_SB_MODE_AUTO);
    lv_obj_align(menu, NULL, 
            LV_ALIGN_IN_TOP_LEFT, 0, CONFIG_JOLT_GUI_STATUSBAR_H);

    /* Create and Stylize Statusbar Title */
    jolt_gui_obj_title_create(parent, title);

    jolt_gui_scr_set_enter_action(parent, jolt_gui_send_enter_main);
    jolt_gui_scr_set_back_action(parent, jolt_gui_scr_del);
    return parent;
}

/* Gets the list object of a menu screen */
lv_obj_t *jolt_gui_scr_menu_get_list(lv_obj_t *par) {
    lv_obj_t *child = NULL;
    lv_obj_type_t obj_type;
    do {
        child = lv_obj_get_child_back(par, child); //the menu should be the first child
        if ( NULL == child ) {
            // cannot find the child list
            return NULL;
        }
        lv_obj_get_type(child, &obj_type);
    } while(strcmp("lv_list", obj_type.type[0]));
    return child;
}

/* Adds an item to a Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_add(lv_obj_t *par, const void *img_src,
        const char *txt, lv_action_t rel_action) {
    lv_obj_t *list = jolt_gui_scr_menu_get_list( par );
    if( NULL == list ) {
        return NULL;
    }
    lv_obj_t *res = lv_list_add(list, img_src, txt, rel_action);

    /* Add the list to the group after the first element has been added
     * so that the first item is properly highlighted. */
    if( NULL == lv_obj_get_group( list ) ) {
        lv_group_add_obj(jolt_gui_store.group.main, list);
        lv_group_focus_obj( list );
    }

    return res;
}
