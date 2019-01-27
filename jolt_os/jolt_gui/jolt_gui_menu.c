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
    JOLT_GUI_CTX{
        lv_obj_t *btn = BREAK_IF_NULL(lv_list_get_btn_selected(list));
        lv_obj_t *label = BREAK_IF_NULL(lv_list_get_btn_label(btn));
        lv_label_set_long_mode(label, mode);
    }
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
        /* When changing selected item, change the label scrolling mode */
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
    JOLT_GUI_SCR_CTX( title ){
        lv_obj_t *menu = BREAK_IF_NULL(lv_list_create(cont_body, NULL));
        if ( NULL == old_list_signal ) {
            old_list_signal = lv_obj_get_signal_func(menu);
        }
        lv_obj_set_signal_func(menu, new_list_signal);
        lv_obj_set_free_num(menu, JOLT_GUI_OBJ_ID_LIST);
        lv_obj_set_size(menu,
                lv_obj_get_width(cont_body), lv_obj_get_height(cont_body));
        lv_list_set_sb_mode(menu, LV_SB_MODE_AUTO);
        lv_page_set_scrl_layout(menu, LV_LAYOUT_COL_L);
        lv_obj_align(menu, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
        jolt_gui_group_add( menu );
        BREAK_IF_NULL(jolt_gui_scr_set_enter_action(parent, jolt_gui_send_enter_main));
        BREAK_IF_NULL(jolt_gui_scr_set_back_action(parent, jolt_gui_scr_del));
    }
    return parent;
}

/* Gets the list object of a menu screen */
lv_obj_t *jolt_gui_scr_menu_get_list(lv_obj_t *parent) {
    lv_obj_t *menu = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *cont_body = NULL;
        cont_body  = JOLT_GUI_FIND_AND_CHECK(parent, JOLT_GUI_OBJ_ID_CONT_BODY);
        menu       = JOLT_GUI_FIND_AND_CHECK(cont_body, JOLT_GUI_OBJ_ID_LIST);
    }
    return menu;
}

/* Adds an item to a Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_add(lv_obj_t *par, const void *img_src,
        const char *txt, lv_action_t rel_action) {
    lv_obj_t *btn = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *list = BREAK_IF_NULL(jolt_gui_scr_menu_get_list( par ));
        btn = BREAK_IF_NULL(lv_list_add(list, img_src, txt, rel_action));
        lv_obj_t *label = BREAK_IF_NULL(lv_list_get_btn_label(btn));
        if( 1 == lv_list_get_size(list) ){
            lv_label_set_long_mode(label, LABEL_LONG_MODE_SELECTED);
        }
        else{
            lv_label_set_long_mode(label, LABEL_LONG_MODE_DEFAULT);
        }
        lv_btn_set_fit(btn, false, false);
    }
    return btn;
}

/* Adds and returns a lv_sw to the specified menu button */
lv_obj_t *jolt_gui_scr_menu_add_sw( lv_obj_t *btn ) {
    lv_obj_t *sw = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *label = lv_list_get_btn_label( btn );
        lv_coord_t width = lv_obj_get_width( label );
        lv_obj_set_width( label, width - JOLT_GUI_SW_CONT_WIDTH );

        lv_obj_t *cont = lv_cont_create(btn, NULL);
        lv_coord_t height = lv_obj_get_height( btn );
        lv_obj_set_protect(cont, LV_PROTECT_POS);
        lv_obj_set_size(cont, JOLT_GUI_SW_CONT_WIDTH, height);
        lv_obj_align(cont, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);

        sw = lv_sw_create( cont, NULL );
        lv_sw_set_anim_time(sw, JOLT_GUI_SW_ANIM_TIME_MS);
        lv_obj_set_size( sw, JOLT_GUI_SW_WIDTH, JOLT_GUI_SW_HEIGHT );
        lv_obj_align(sw, NULL, LV_ALIGN_IN_LEFT_MID, JOLT_GUI_SW_LEFT_PADDING, 0);
    }
    return sw;
}

void jolt_gui_scr_menu_set_btn_selected(lv_obj_t *par, lv_obj_t *btn){
    JOLT_GUI_CTX{
        lv_obj_t *list = BREAK_IF_NULL(jolt_gui_scr_menu_get_list(par));
        set_selected_label_long_mode(list, LABEL_LONG_MODE_DEFAULT);
        uint16_t anim_time = lv_list_get_anim_time(list);
        lv_list_set_anim_time(list, 0);
        lv_list_set_btn_selected(list, btn);
        lv_list_set_anim_time(list, anim_time);
        set_selected_label_long_mode(list, LABEL_LONG_MODE_SELECTED);
    }
}
