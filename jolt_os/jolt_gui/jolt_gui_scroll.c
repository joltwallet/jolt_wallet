#include "jolt_gui.h"

/* Scrollable Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--PAGE
 *           +--All additional objects
 */

static const char TAG[] = "jolt_gui_scroll";
static lv_style_t monospace_style_obj;
static lv_style_t *monospace_style = NULL;

#define CHECK_SCR(x) if( JOLT_GUI_SCR_ID_SCROLL != jolt_gui_scr_id_get(x) ) abort();

jolt_gui_obj_t *jolt_gui_scr_scroll_get_page( jolt_gui_obj_t *scr )
{
    jolt_gui_obj_t *page = NULL;
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR(scr);

        jolt_gui_obj_t *cont_body = JOLT_GUI_FIND_AND_CHECK( scr, JOLT_GUI_OBJ_ID_CONT_BODY );
        page                      = JOLT_GUI_FIND_AND_CHECK( cont_body, JOLT_GUI_OBJ_ID_PAGE );
    }
    return page;
}

jolt_gui_obj_t *jolt_gui_scr_scroll_create( const char *title )
{
    JOLT_GUI_SCR_CTX( title )
    {
        /* Set screen ID */
        jolt_gui_scr_id_set( parent, JOLT_GUI_SCR_ID_SCROLL );

        /* Create Scrollable Page */
        jolt_gui_obj_t *page = BREAK_IF_NULL( lv_page_create( cont_body, NULL ) );
        jolt_gui_obj_id_set( page, JOLT_GUI_OBJ_ID_PAGE );
        lv_obj_set_size( page, LV_HOR_RES, LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H );
        lv_obj_align( page, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0 );
        lv_page_set_sb_mode( page, LV_SB_MODE_AUTO );
        lv_page_set_scrl_layout( page, LV_LAYOUT_COL_M );

        jolt_gui_group_add( page );
        lv_group_focus_obj( page );
        jolt_gui_scr_set_event_cb( parent, jolt_gui_event_del ); /* By default, back just deletes screen */
    }
    return parent;
}

/* Add a text container to a scrollable screen */
jolt_gui_obj_t *jolt_gui_scr_scroll_add_text( jolt_gui_obj_t *scr, const char *text )
{
    jolt_gui_obj_t *label = NULL;
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR(scr);

        jolt_gui_obj_t *page = BREAK_IF_NULL( jolt_gui_scr_scroll_get_page( scr ) );
        /* Create Text Label on Page */
        label = BREAK_IF_NULL( lv_label_create( page, NULL ) );
        lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
        lv_obj_set_width( label, lv_page_get_fit_width( page ) );
        lv_label_set_align( label, LV_LABEL_ALIGN_LEFT );
        lv_label_set_text( label, text );
    }
    return label;
}

/* Add a monospace text container to a scrollable screen */
jolt_gui_obj_t *jolt_gui_scr_scroll_add_monospace_text( jolt_gui_obj_t *scr, const char *text )
{
    jolt_gui_obj_t *label = NULL;
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR(scr);

        jolt_gui_obj_t *page = BREAK_IF_NULL( jolt_gui_scr_scroll_get_page( scr ) );
        /* Create Text Label on Page */
        label = BREAK_IF_NULL( lv_label_create( page, NULL ) );
        lv_label_set_long_mode( label, LV_LABEL_LONG_BREAK );
        lv_obj_set_width( label, lv_page_get_fit_width( page ) );

        if( NULL == monospace_style ) {
            monospace_style = &monospace_style_obj;
            lv_style_copy( monospace_style, lv_label_get_style( label, LV_LABEL_STYLE_MAIN ) );
            monospace_style->text.font = &unscii_8;
        }
        lv_label_set_style( label, LV_LABEL_STYLE_MAIN, monospace_style );
        lv_label_set_align( label, LV_LABEL_ALIGN_LEFT );

        lv_label_set_text( label, text );
    }
    return label;
}
