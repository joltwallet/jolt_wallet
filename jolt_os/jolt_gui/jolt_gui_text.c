#include "jolt_gui.h"

/* Text Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--LABEL_0
 */

/* Creates a simple screen with a single scrollable text body
 * */
lv_obj_t *jolt_gui_scr_text_create(const char *title, const char *body) {
    JOLT_GUI_SCR_PREAMBLE( title );

    /* Create Scrollable Page */
    lv_obj_t *page = lv_page_create(cont_body, NULL);
    JOLT_GUI_OBJ_CHECK( page );
    lv_obj_set_free_num(page, JOLT_GUI_OBJ_ID_PAGE);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H);
    lv_obj_align(page, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_page_set_sb_mode(page, LV_SB_MODE_AUTO);
    jolt_gui_group_add( page );
    lv_page_set_arrow_scroll(page, true);

    /* Create Text Label on Page */
    lv_obj_t *label = lv_label_create(page, NULL);
    JOLT_GUI_OBJ_CHECK( label );
    lv_obj_set_free_num(label, JOLT_GUI_OBJ_ID_LABEL_0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    {
        // Compute Width of Label (don't draw into scrollbar)
        lv_style_t *sb = lv_page_get_style(page, LV_PAGE_STYLE_SB);
        lv_obj_set_width(label, lv_page_get_scrl_width(page) - 
                sb->body.padding.inner - sb->body.padding.hor);  
    }
    lv_label_set_text(label, body);

    lv_group_focus_obj(page);

    jolt_gui_scr_set_back_action(parent, jolt_gui_scr_del);
    jolt_gui_scr_set_enter_action(parent, NULL);

exit:
    return parent;
}

