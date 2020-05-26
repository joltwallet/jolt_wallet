//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "jolt_gui_loading.h"
#include "esp_log.h"
#include "jolt_gui.h"

/* Loading and Preloading Screen Structures:
 * * SCREEN
 *   +--LABEL_TITLE
 *   +--CONT_BODY
 *       +--LOADINGBAR or PRELOADING
 *       +--LABEL_0 (loading label)
 */

static const char TAG[] = "scr_loading";

#define CHECK_SCR_LOADINGBAR( x ) \
    if( JOLT_GUI_SCR_ID_LOADINGBAR != jolt_gui_scr_id_get( x ) ) abort();
#define CHECK_SCR_PRELOADING( x ) \
    if( JOLT_GUI_SCR_ID_PRELOADING != jolt_gui_scr_id_get( x ) ) abort();

static lv_signal_cb_t old_bar_signal = NULL; /*Store the old signal function*/

typedef struct autoupdate_param_t {
    jolt_gui_obj_t *scr;
    lv_task_t *task;
    int8_t progress;
    uint8_t apply_sent;
} autoupdate_param_t;

typedef struct {
    lv_bar_ext_t cont; /*The ancestor container structure*/
    autoupdate_param_t *autoupdate;
} loadingbar_ext_t;

static lv_res_t new_bar_signal( jolt_gui_obj_t *bar, lv_signal_t sign, void *param )
{
    lv_res_t res;

    loadingbar_ext_t *ext = lv_obj_get_ext_attr( bar );

    if( sign == LV_SIGNAL_CLEANUP ) {
        if( NULL != ext->autoupdate ) {
            lv_task_del( ext->autoupdate->task );
            free( ext->autoupdate );
        }
    }

    res = old_bar_signal( bar, sign, param );
    if( res != LV_RES_OK ) return res;

    return res;
}


void jolt_gui_scr_loadingbar_update( jolt_gui_obj_t *parent, const char *title, const char *text, int8_t percentage )
{
    JOLT_GUI_CTX
    {
        /* Make sure that the parent is a screen */
        parent = jolt_gui_scr_get( parent );
        CHECK_SCR_LOADINGBAR( parent );

        /* Find Objects */
        jolt_gui_obj_t *label_title = NULL;
        {
            jolt_gui_obj_t *cont_title = NULL;
            cont_title                 = JOLT_GUI_FIND_AND_CHECK( parent, JOLT_GUI_OBJ_ID_CONT_TITLE );
            label_title                = JOLT_GUI_FIND_AND_CHECK( cont_title, JOLT_GUI_OBJ_ID_LABEL_0 );
        }

        jolt_gui_obj_t *bar_loading   = NULL;
        jolt_gui_obj_t *label_loading = NULL;
        {
            jolt_gui_obj_t *cont_body = NULL;
            cont_body                 = JOLT_GUI_FIND_AND_CHECK( parent, JOLT_GUI_OBJ_ID_CONT_BODY );
            bar_loading               = JOLT_GUI_FIND_AND_CHECK( cont_body, JOLT_GUI_OBJ_ID_LOADINGBAR );
            label_loading             = JOLT_GUI_FIND_AND_CHECK( cont_body, JOLT_GUI_OBJ_ID_LABEL_0 );
        }

        if( percentage > 100 ) { percentage = 100; }
        int16_t current_val = lv_bar_get_value( bar_loading );
        if( current_val != percentage && percentage >= 0 ) {
            ESP_LOGD( TAG, "(%d) Progress: %d", __LINE__, percentage );
            lv_bar_set_value( bar_loading, percentage, LV_ANIM_ON );
            loadingbar_ext_t *ext = lv_obj_get_ext_attr( bar_loading );
            if( ext->autoupdate ) { ext->autoupdate->progress = percentage; }
            lv_event_send( bar_loading, jolt_gui_event.value_changed, NULL );
        }
        if( text && strcmp(lv_label_get_text(label_loading), text)) {
            lv_label_set_text( label_loading, text ); 
        }
        if( title && strcmp(lv_label_get_text(label_title), text)) { 
            lv_label_set_text( label_title, title ); 
        }
    }
    taskYIELD();  // This doesn't seem sufficient to let the screen fully draw.
}

/* lv_task that periodically updates the loading screen */
static void autoupdate_task( lv_task_t *input )
{
    jolt_gui_obj_t *scr = input->user_data;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *bar   = BREAK_IF_NULL( jolt_gui_scr_get_active( scr ) );
        loadingbar_ext_t *ext = lv_obj_get_ext_attr( bar );
        if( ext->autoupdate->progress < 100 && ext->autoupdate->progress >= 0 ) {
            /* Reset the apply_sent flag if the percentage is a valid range
             * and its not at 100% */
            ext->autoupdate->apply_sent = false;
        }
        if( ext->autoupdate->progress <= 100 && ext->autoupdate->progress >= 0 ) {
            jolt_gui_scr_loadingbar_update( scr, NULL, NULL, ext->autoupdate->progress );
        }
        if( !ext->autoupdate->apply_sent && ext->autoupdate->progress >= 100 ) { 
            lv_event_send( bar, jolt_gui_event.apply, NULL ); 
            ext->autoupdate->apply_sent = true;
        }
    }
}

int8_t *jolt_gui_scr_loadingbar_autoupdate( jolt_gui_obj_t *parent )
{
    autoupdate_param_t *param;
    param = calloc( 1, sizeof( autoupdate_param_t ) );
    if( NULL == param ) {
        ESP_LOGE( TAG, "Unable to allocate memory for autoupdate params" );
        return NULL;
    }
    ESP_LOGD( TAG, "%d: Enabling autoupdate on screen %p", __LINE__, parent );

    JOLT_GUI_CTX
    {
        /* Make sure that the parent is a screen */
        parent = jolt_gui_scr_get( parent );
        CHECK_SCR_LOADINGBAR( parent );

        jolt_gui_obj_t *bar   = BREAK_IF_NULL( jolt_gui_scr_get_active( parent ) );
        loadingbar_ext_t *ext = lv_obj_get_ext_attr( bar );
        ext->autoupdate       = param;
        param->scr            = parent;
        param->progress       = 0;
        param->task           = lv_task_create( autoupdate_task, 100, LV_TASK_PRIO_HIGH, parent );
    }
    if( NULL == param->task ) {
        free( param );
        return NULL;
    }
    ESP_LOGD( TAG, "Autoupdate create progress address: %p", &param->progress );
    return &param->progress;
}

/* Creates and returns a loading screen.
 * Defaults the back and enter action to NULL.
 * */
jolt_gui_obj_t *jolt_gui_scr_loadingbar_create( const char *title )
{
    JOLT_GUI_SCR_CTX( title )
    {
        /* Set screen ID */
        jolt_gui_scr_id_set( parent, JOLT_GUI_SCR_ID_LOADINGBAR );
        ESP_LOGD( TAG, "%d: Created loadingbar screen %p", __LINE__, parent );

        /* Create Loading Bar */
        jolt_gui_obj_t *bar = BREAK_IF_NULL( lv_bar_create( cont_body, NULL ) );
        BREAK_IF_NULL( lv_obj_allocate_ext_attr( bar, sizeof( loadingbar_ext_t ) ) );
        loadingbar_ext_t *ext = lv_obj_get_ext_attr( bar );
        ESP_LOGD( TAG, "loadingbar ext: %p", ext );
        ext->autoupdate = NULL;
        if( NULL == old_bar_signal ) { old_bar_signal = lv_obj_get_signal_cb( bar ); /*Save to old signal function*/ }
        lv_obj_set_signal_cb( bar, new_bar_signal );
        jolt_gui_obj_id_set( bar, JOLT_GUI_OBJ_ID_LOADINGBAR );
        jolt_gui_group_add( bar );
        lv_obj_set_size( bar, CONFIG_JOLT_GUI_LOADINGBAR_W, CONFIG_JOLT_GUI_LOADINGBAR_H );
        lv_obj_align( bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10 );
        lv_bar_set_anim_time( bar, CONFIG_JOLT_GUI_LOADINGBAR_ANIM_MS );
        lv_bar_set_value( bar, 1, LV_ANIM_ON );
        ESP_LOGD( TAG, "%d: Created loadingbar %p", __LINE__, bar );

        /* Create Loading Label */
        jolt_gui_obj_t *label = BREAK_IF_NULL( lv_label_create( cont_body, NULL ) );
        jolt_gui_obj_id_set( label, JOLT_GUI_OBJ_ID_LABEL_0 );
        lv_label_set_align( label, LV_LABEL_ALIGN_CENTER );
        lv_label_set_text( label, "Initializing" );
        lv_label_set_long_mode( label, LV_LABEL_LONG_SROLL );

        /* Set Label Size and Position */
        const lv_style_t *label_style = lv_obj_get_style( label );
        lv_obj_set_size( label, lv_obj_get_width( cont_body ), label_style->text.font->line_height );
        lv_obj_align( label, bar, LV_ALIGN_OUT_TOP_MID, 0, -10 );

        jolt_gui_scr_set_event_cb( parent, NULL );
    }
    return parent;
}

int8_t *jolt_gui_scr_loadingbar_progress_get( lv_obj_t *obj )
{
    int8_t *progress = NULL;
    JOLT_GUI_CTX
    {
        jolt_gui_obj_t *scr = jolt_gui_scr_get( obj );
        CHECK_SCR_LOADINGBAR( scr );
        jolt_gui_obj_t *bar   = BREAK_IF_NULL( jolt_gui_scr_get_active( scr ) );
        loadingbar_ext_t *ext = lv_obj_get_ext_attr( bar );
        if( NULL == ext->autoupdate ) break;
        progress = &ext->autoupdate->progress;
    }
    return progress;
}

/* Creates and returns a preloading screen.
 * A preloading screen has no progression and is used for tasks of unknown
 * lenth and/or progression.
 *
 * Defaults the back and enter action to NULL.
 * */
jolt_gui_obj_t *jolt_gui_scr_preloading_create( const char *title, const char *text )
{
    JOLT_GUI_SCR_CTX( title )
    {
        /* Set screen ID */
        jolt_gui_scr_id_set( parent, JOLT_GUI_SCR_ID_PRELOADING );

        /* Create PreLoading Object */
        jolt_gui_obj_t *preload = BREAK_IF_NULL( lv_preload_create( cont_body, NULL ) );
        jolt_gui_obj_id_set( preload, JOLT_GUI_OBJ_ID_PRELOADING );
        jolt_gui_group_add( preload );
        lv_obj_set_size( preload, 30, 30 );
        lv_obj_align( preload, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0 );

        /* Create Loading Label */
        jolt_gui_obj_t *label = BREAK_IF_NULL( lv_label_create( cont_body, NULL ) );
        jolt_gui_obj_id_set( label, JOLT_GUI_OBJ_ID_LABEL_0 );
        lv_label_set_align( label, LV_LABEL_ALIGN_CENTER );
        if( NULL == text ) { lv_label_set_text( label, EMPTY_STR ); }
        else {
            lv_label_set_text( label, text );
        }
        lv_label_set_long_mode( label, LV_LABEL_LONG_SROLL );
        const lv_style_t *label_style = lv_obj_get_style( label );
        lv_obj_set_size( label, lv_obj_get_width( cont_body ), label_style->text.font->line_height );
        lv_obj_align( label, preload, LV_ALIGN_OUT_TOP_MID, 0, -6 );

        jolt_gui_scr_set_event_cb( parent, NULL );
    }
    return parent;
}

void jolt_gui_scr_preloading_update( jolt_gui_obj_t *parent, const char *title, const char *text )
{
    JOLT_GUI_CTX
    {
        /* Make sure that the parent is a screen */
        parent = jolt_gui_scr_get( parent );
        CHECK_SCR_PRELOADING( parent );

        /* Find Objects */
        jolt_gui_obj_t *label_title = NULL;
        {
            jolt_gui_obj_t *cont_title = NULL;
            cont_title                 = JOLT_GUI_FIND_AND_CHECK( parent, JOLT_GUI_OBJ_ID_CONT_TITLE );
            label_title                = JOLT_GUI_FIND_AND_CHECK( cont_title, JOLT_GUI_OBJ_ID_LABEL_0 );
        }

        jolt_gui_obj_t *label_loading = NULL;
        {
            jolt_gui_obj_t *cont_body = NULL;
            cont_body                 = JOLT_GUI_FIND_AND_CHECK( parent, JOLT_GUI_OBJ_ID_CONT_BODY );
            label_loading             = JOLT_GUI_FIND_AND_CHECK( cont_body, JOLT_GUI_OBJ_ID_LABEL_0 );
        }

        if( text ) { lv_label_set_text( label_loading, text ); }
        if( title ) { lv_label_set_text( label_title, title ); }
    }
}
