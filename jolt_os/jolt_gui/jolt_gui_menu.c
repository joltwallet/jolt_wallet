//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "jolt_gui_menu.h"
#include "esp_log.h"
#include "jolt_gui.h"

/* Menu Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--LIST
 */
#define LABEL_LONG_MODE_DEFAULT  LV_LABEL_LONG_DOT
#define LABEL_LONG_MODE_SELECTED LV_LABEL_LONG_SROLL
static const char TAG[] = "scr_menu";

#define CHECK_SCR( x ) \
    if( JOLT_GUI_SCR_ID_MENU != jolt_gui_scr_id_get( x ) ) abort();

static lv_signal_cb_t old_list_signal = NULL; /*Store the old signal function*/

static void set_selected_label_long_mode( jolt_gui_obj_t *list, lv_label_long_mode_t mode )
{
    JOLT_GUI_CTX
    {
        uint16_t len = lv_list_get_size( list );
        if( len > 0 ) {
            jolt_gui_obj_t *btn   = BREAK_IF_NULL( lv_list_get_btn_selected( list ) );
            jolt_gui_obj_t *label = BREAK_IF_NULL( lv_list_get_btn_label( btn ) );
            lv_label_set_long_mode( label, mode );
        }
    }
}

static lv_res_t new_list_signal( jolt_gui_obj_t *list, lv_signal_t sign, void *param )
{
    lv_res_t res;
    char c;

    if( sign == LV_SIGNAL_FOCUS ) {
        // call ancestor
        res = old_list_signal( list, sign, param );
        if( res != LV_RES_OK ) return res;
        set_selected_label_long_mode( list, LABEL_LONG_MODE_SELECTED );
    }
    else if( sign == LV_SIGNAL_CONTROL && ( c = *( (char *)param ), ( c == LV_KEY_RIGHT || c == LV_KEY_DOWN ||
                                                                      c == LV_KEY_LEFT || c == LV_KEY_UP ) ) ) {
        /* When changing selected item, change the label scrolling mode */
        set_selected_label_long_mode( list, LABEL_LONG_MODE_DEFAULT );
        res = old_list_signal( list, sign, param );
        if( res != LV_RES_OK ) return res;
        set_selected_label_long_mode( list, LABEL_LONG_MODE_SELECTED );
    }
    else {
        res = old_list_signal( list, sign, param );
        if( res != LV_RES_OK ) return res;
    }

    return res;
}

/* Creates a standard Jolt Menu Screen */
jolt_gui_obj_t *jolt_gui_scr_menu_create( const char *title )
{
    JOLT_GUI_SCR_CTX( title )
    {
        jolt_gui_scr_id_set( parent, JOLT_GUI_SCR_ID_MENU );
        jolt_gui_obj_t *menu = BREAK_IF_NULL( lv_list_create( cont_body, NULL ) );
        ESP_LOGD( TAG, "Menu created %p", menu );
        if( NULL == old_list_signal ) { old_list_signal = lv_obj_get_signal_cb( menu ); }
        const lv_style_t *sb_style = lv_obj_get_style( menu );

        lv_obj_set_signal_cb( menu, new_list_signal );

        jolt_gui_obj_id_set( menu, JOLT_GUI_OBJ_ID_LIST );
        lv_obj_set_size( menu, lv_obj_get_width( cont_body ) - sb_style->body.padding.inner,
                         lv_obj_get_height( cont_body ) );
        lv_list_set_sb_mode( menu, LV_SB_MODE_AUTO );
        lv_page_set_scrl_layout( menu, LV_LAYOUT_COL_L );
        lv_obj_align( menu, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0 );

        jolt_gui_group_add( menu );
        jolt_gui_scr_set_event_cb( parent, jolt_gui_event_del );
    }
    return parent;
}

/* Gets the list object of a menu screen */
jolt_gui_obj_t *jolt_gui_scr_menu_get_list( const jolt_gui_obj_t *par )
{
    jolt_gui_obj_t *menu = NULL;
    JOLT_GUI_CTX
    {
        /* Make sure that the par is a screen */
        par = jolt_gui_scr_get( par );
        CHECK_SCR( par );

        jolt_gui_obj_t *cont_body = NULL;
        cont_body                 = JOLT_GUI_FIND_AND_CHECK( par, JOLT_GUI_OBJ_ID_CONT_BODY );
        menu                      = JOLT_GUI_FIND_AND_CHECK( cont_body, JOLT_GUI_OBJ_ID_LIST );
    }
    return menu;
}

jolt_gui_obj_t *jolt_gui_scr_menu_get_scr( jolt_gui_obj_t *btn )
{
    jolt_gui_obj_t *scr = NULL;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *list = lv_obj_get_parent( lv_obj_get_parent( btn ) );
        scr                  = lv_obj_get_parent( lv_obj_get_parent( list ) );
    }
    return scr;
}

/* Adds an item to a Jolt Menu Screen */
jolt_gui_obj_t *jolt_gui_scr_menu_add( jolt_gui_obj_t *par, const void *img_src, const char *txt,
                                       lv_event_cb_t event_cb )
{
    jolt_gui_obj_t *btn = NULL;
    JOLT_GUI_CTX
    {
        /* Make sure that the par is a screen */
        par = jolt_gui_scr_get( par );
        CHECK_SCR( par );

        jolt_gui_obj_t *list = BREAK_IF_NULL( jolt_gui_scr_menu_get_list( par ) );
        btn                  = BREAK_IF_NULL( lv_list_add_btn( list, img_src, txt ) );
        lv_obj_set_event_cb( btn, event_cb );
        jolt_gui_obj_t *label = BREAK_IF_NULL( lv_list_get_btn_label( btn ) );
        if( 1 == lv_list_get_size( list ) ) { lv_label_set_long_mode( label, LABEL_LONG_MODE_SELECTED ); }
        else {
            lv_label_set_long_mode( label, LABEL_LONG_MODE_DEFAULT );
        }
        lv_btn_set_fit2( btn, false, false );
    }
    return btn;
}

/* Adds and returns a lv_sw to the specified menu button */
jolt_gui_obj_t *jolt_gui_scr_menu_add_sw( jolt_gui_obj_t *btn )
{
    jolt_gui_obj_t *sw = NULL;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *label = lv_list_get_btn_label( btn );
        lv_coord_t width      = lv_obj_get_width( label );
        lv_obj_set_width( label, width - JOLT_GUI_SW_CONT_WIDTH );

        jolt_gui_obj_t *cont = BREAK_IF_NULL( lv_cont_create( btn, NULL ) );
        lv_coord_t height    = lv_obj_get_height( btn );
        lv_obj_set_protect( cont, LV_PROTECT_POS );
        lv_obj_set_size( cont, JOLT_GUI_SW_CONT_WIDTH, height );
        lv_obj_align( cont, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0 );

        sw = BREAK_IF_NULL( lv_sw_create( cont, NULL ) );
        lv_sw_set_anim_time( sw, JOLT_GUI_SW_ANIM_TIME_MS );
        lv_obj_set_size( sw, JOLT_GUI_SW_WIDTH, JOLT_GUI_SW_HEIGHT );
        lv_obj_align( sw, NULL, LV_ALIGN_IN_LEFT_MID, JOLT_GUI_SW_LEFT_PADDING, 0 );
    }
    return sw;
}

lv_obj_t *jolt_gui_scr_menu_add_info( jolt_gui_obj_t *btn, const char *info )
{
    jolt_gui_obj_t *info_label = NULL;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *info_label_tmp, *btn_label;
        btn_label = lv_list_get_btn_label( btn );

        /* Creating another child label under a list element interfere's with
         * how lvgl handles signal callbacks, so we have to first create a container */
        jolt_gui_obj_t *cont = BREAK_IF_NULL( lv_cont_create( btn, NULL ) );
        lv_cont_set_style( cont, LV_CONT_STYLE_MAIN, &lv_style_transp_tight );
        lv_cont_set_fit( cont, LV_FIT_TIGHT );

        info_label_tmp = BREAK_IF_NULL( lv_label_create( cont, btn_label ) );
        lv_label_set_long_mode( info_label_tmp, LV_LABEL_LONG_EXPAND );
        lv_label_set_text( info_label_tmp, info );

        {
            lv_coord_t info_width = lv_obj_get_width( cont );
            lv_coord_t btn_width  = lv_obj_get_width( btn_label );
            lv_obj_set_width( btn_label, btn_width - info_width );
            lv_obj_align( btn_label, btn, LV_ALIGN_IN_LEFT_MID, 0, 0 );
        }

        lv_obj_align( cont, btn_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0 );
        info_label = info_label_tmp;
    }
    return info_label;
}

void jolt_gui_scr_menu_set_btn_selected( jolt_gui_obj_t *par, jolt_gui_obj_t *btn )
{
    JOLT_GUI_CTX
    {
        /* Make sure that the par is a screen */
        par = jolt_gui_scr_get( par );
        CHECK_SCR( par );

        jolt_gui_obj_t *list = BREAK_IF_NULL( jolt_gui_scr_menu_get_list( par ) );
        set_selected_label_long_mode( list, LABEL_LONG_MODE_DEFAULT );
        uint16_t anim_time = lv_list_get_anim_time( list );
        lv_list_set_anim_time( list, 0 );
        lv_list_set_btn_selected( list, btn );
        lv_list_set_anim_time( list, anim_time );
        set_selected_label_long_mode( list, LABEL_LONG_MODE_SELECTED );
    }
}

void jolt_gui_scr_menu_remove( lv_obj_t *par, lv_obj_t *btn )
{
    JOLT_GUI_CTX
    {
        /* Make sure that the par is a screen */
        par = jolt_gui_scr_get( par );
        CHECK_SCR( par );

        jolt_gui_obj_t *list = BREAK_IF_NULL( jolt_gui_scr_menu_get_list( par ) );
        int32_t index        = lv_list_get_btn_index( list, btn );
        lv_list_remove( list, index );
    }
}

void jolt_gui_scr_menu_remove_indices( jolt_gui_obj_t *par, uint16_t start, uint16_t end )
{
    if( 0 == end ) { end = UINT16_MAX; }
    else {
        end += 1;
    }

    JOLT_GUI_CTX
    {
        /* Make sure that the par is a screen */
        par = jolt_gui_scr_get( par );
        CHECK_SCR( par );

        jolt_gui_obj_t *list = BREAK_IF_NULL( jolt_gui_scr_menu_get_list( par ) );
        for( uint16_t i = start; i < end; i++ ) {
            bool res = lv_list_remove( list, start );
            if( !res ) { break; }
        }
    }
}

bool jolt_gui_scr_menu_set_param( jolt_gui_obj_t *par, void *param )
{
    bool res = false;
    JOLT_GUI_CTX
    {
        /* Make sure that the par is a screen */
        par = jolt_gui_scr_get( par );
        CHECK_SCR( par );

        jolt_gui_obj_t *list, *btn;
        list = BREAK_IF_NULL( jolt_gui_scr_menu_get_list( par ) );
        jolt_gui_obj_set_param( list, param );
        btn = NULL;
        while( NULL != ( btn = lv_list_get_next_btn( list, btn ) ) ) { jolt_gui_obj_set_param( btn, param ); }
        res = true;
    }
    return res;
}

int32_t jolt_gui_scr_menu_get_btn_index( jolt_gui_obj_t *btn )
{
    int32_t index = -1;
    JOLT_GUI_CTX { index = lv_list_get_btn_index( NULL, btn ); }
    return index;
}

uint16_t jolt_gui_scr_menu_len( const lv_obj_t *par )
{
    uint16_t len = 0;
    JOLT_GUI_CTX
    {
        /* Make sure that the par is a screen */
        par = jolt_gui_scr_get( par );
        CHECK_SCR( par );

        jolt_gui_obj_t *list = BREAK_IF_NULL( jolt_gui_scr_menu_get_list( par ) );
        len                  = lv_list_get_size( list );
    }
    return len;
}
