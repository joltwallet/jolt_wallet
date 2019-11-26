#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "jolt_gui_slider.h"
#include "esp_log.h"
#include "jolt_gui.h"

/* Slider Screen Structure:
 * * SCREEN
 *   +--CONT_TITLE
 *   |   +--LABEL_0 (title)
 *   +--CONT_BODY
 *       +--SLIDER
 *            +--LABEL_0
 */

static const char TAG[] = "scr_slider";

#define CHECK_SCR(x) if( JOLT_GUI_SCR_ID_SLIDER != jolt_gui_scr_id_get(x) ) abort();

jolt_gui_obj_t *jolt_gui_scr_slider_get_slider( jolt_gui_obj_t *parent )
{
    jolt_gui_obj_t *slider = NULL;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *cont_body = NULL;
        cont_body                 = JOLT_GUI_FIND_AND_CHECK( parent, JOLT_GUI_OBJ_ID_CONT_BODY );
        slider                    = JOLT_GUI_FIND_AND_CHECK( cont_body, JOLT_GUI_OBJ_ID_SLIDER );
    }
    return slider;
}

int16_t jolt_gui_scr_slider_get_value( jolt_gui_obj_t *scr )
{
    uint16_t val = 0;
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR(scr);

        jolt_gui_obj_t *slider = BREAK_IF_NULL( jolt_gui_scr_slider_get_slider( scr ) );
        val                    = lv_slider_get_value( slider );
    }
    return val;
}

void jolt_gui_scr_slider_set_value( jolt_gui_obj_t *scr, int16_t value )
{
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR(scr);

        jolt_gui_obj_t *slider = BREAK_IF_NULL( jolt_gui_scr_slider_get_slider( scr ) );
        lv_slider_set_value( slider, value, false );
    }
}

void jolt_gui_scr_slider_set_range( jolt_gui_obj_t *scr, int16_t min, int16_t max )
{
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR(scr);

        jolt_gui_obj_t *slider = BREAK_IF_NULL( jolt_gui_scr_slider_get_slider( scr ) );
        lv_slider_set_range( slider, min, max );
    }
}

void jolt_gui_scr_slider_set_label( jolt_gui_obj_t *scr, const char *text )
{
    JOLT_GUI_CTX
    {
        /* Make sure that the scr is a screen */
        scr = jolt_gui_scr_get( scr );
        CHECK_SCR(scr);

        jolt_gui_obj_t *slider = BREAK_IF_NULL( jolt_gui_scr_slider_get_slider( scr ) );
        jolt_gui_obj_t *label  = NULL;
        label                  = JOLT_GUI_FIND_AND_CHECK( slider, JOLT_GUI_OBJ_ID_LABEL_0 );
        lv_label_set_text( label, text );
    }
}

jolt_gui_obj_t *jolt_gui_scr_slider_create( const char *title, const char *text, jolt_gui_event_cb_t cb )
{
    JOLT_GUI_SCR_CTX( title )
    {
        jolt_gui_scr_id_set( parent, JOLT_GUI_SCR_ID_SLIDER );
        /* Create Slider */
        jolt_gui_obj_t *slider = BREAK_IF_NULL( lv_slider_create( cont_body, NULL ) );
        jolt_gui_obj_id_set( slider, JOLT_GUI_OBJ_ID_SLIDER );
        lv_slider_set_range( slider, 0, 10 );  // Default Slider Range
        lv_obj_set_size( slider, CONFIG_JOLT_GUI_SLIDER_W, CONFIG_JOLT_GUI_SLIDER_H );
        lv_obj_align( slider, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10 );
        jolt_gui_obj_set_event_cb( slider, cb );
        lv_slider_set_value( slider, 0, false );  // Default initial value
        lv_slider_set_anim_time( slider, CONFIG_JOLT_GUI_SLIDER_ANIM_MS );

        jolt_gui_group_add( slider );

        /* Create Label */
        jolt_gui_obj_t *label = BREAK_IF_NULL( lv_label_create( slider, NULL ) );
        jolt_gui_obj_id_set( label, JOLT_GUI_OBJ_ID_LABEL_0 );
        lv_label_set_align( label, LV_LABEL_ALIGN_CENTER );
        if( NULL == text ) { lv_label_set_text( label, EMPTY_STR ); }
        else {
            lv_label_set_text( label, text );
        }
        lv_label_set_long_mode( label, LV_LABEL_LONG_SROLL );
        const lv_style_t *label_style = lv_obj_get_style( label );
        lv_obj_set_size( label, lv_obj_get_width( cont_body ), label_style->text.font->line_height );
        lv_obj_align( label, NULL, LV_ALIGN_OUT_TOP_MID, 0, -6 );
    }
    return parent;
}
