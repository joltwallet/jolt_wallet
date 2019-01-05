#include "jolt_gui_menu.h"
#include "jolt_gui.h"

/* Menu Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--LIST
 */
#define LABEL_LONG_MODE_DEFAULT LV_LABEL_LONG_DOT
#define LABEL_LONG_MODE_SELECTED LV_LABEL_LONG_ROLL 
static const char TAG[] = "scr_menu";

static lv_signal_func_t old_list_signal = NULL;     /*Store the old signal function*/

static void set_selected_label_long_mode(lv_obj_t *list, lv_label_long_mode_t mode){
    lv_obj_t *btn = lv_list_get_btn_selected(list);
    if( NULL == btn ){
        return;
    }
    lv_obj_t *label = lv_list_get_btn_label(btn);
    if( NULL == label ){
        return;
    }
    lv_label_set_long_mode(label, mode);
}

static lv_res_t new_list_signal(lv_obj_t *list, lv_signal_t sign, void * param)
{
    lv_res_t res;
    char c;
    
    if(sign == LV_SIGNAL_FOCUS) {
        // call ancestor
        res = old_list_signal(list, sign, param);
        if(res != LV_RES_OK) return res;
        set_selected_label_long_mode(list, LABEL_LONG_MODE_SELECTED);
    }
    else if(sign == LV_SIGNAL_CONTROLL
                && (c = *((char *)param), 
                        (c == LV_GROUP_KEY_RIGHT || c == LV_GROUP_KEY_DOWN
                        || c == LV_GROUP_KEY_LEFT || c == LV_GROUP_KEY_UP)) ){
        set_selected_label_long_mode(list, LABEL_LONG_MODE_DEFAULT);
        res = old_list_signal(list, sign, param);
        if(res != LV_RES_OK) return res;
        set_selected_label_long_mode(list, LABEL_LONG_MODE_SELECTED);
    }
    else{
        res = old_list_signal(list, sign, param);
        if(res != LV_RES_OK) return res;
    }

    return res;
}

/* Creates a standard Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_create(const char *title) {
    JOLT_GUI_SCR_PREAMBLE( title );

    /* Create List */
    // todo, get these values from parent
    lv_obj_t *menu = lv_list_create(cont_body, NULL);
    JOLT_GUI_OBJ_CHECK(menu);
    if ( NULL == old_list_signal ) {
        old_list_signal = lv_obj_get_signal_func(menu);
    }
    lv_obj_set_signal_func(menu, new_list_signal);
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
    lv_obj_t *list = jolt_gui_scr_menu_get_list( par );
    if( NULL == list ) {
        return NULL;
    }

    lv_obj_t *btn = lv_list_add(list, img_src, txt, rel_action);
    if( NULL == btn ){
        goto exit;
    }

    lv_obj_t *label = lv_list_get_btn_label(btn);
    if( NULL == btn ){
        goto exit;
    }

    if( 1 == lv_list_get_size(list) ){
        lv_label_set_long_mode(label, LABEL_LONG_MODE_SELECTED);
    }
    else{
        lv_label_set_long_mode(label, LABEL_LONG_MODE_DEFAULT);
    }

exit:
    return btn;
}

void jolt_gui_scr_menu_set_btn_selected(lv_obj_t *par, lv_obj_t *btn){
    lv_obj_t *list = jolt_gui_scr_menu_get_list(par);
    set_selected_label_long_mode(list, LABEL_LONG_MODE_DEFAULT);
    uint16_t anim_time = lv_list_get_anim_time(list);
    lv_list_set_anim_time(list, 0);
    lv_list_set_btn_selected(list, btn);
    lv_list_set_anim_time(list, anim_time);
    set_selected_label_long_mode(list, LABEL_LONG_MODE_SELECTED);
}
