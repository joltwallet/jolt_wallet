//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "jolt_gui.h"
#include "esp_log.h"
#include "stdio.h"

/*********************
 *      DEFINES
 *********************/
#define ID_MASK     0x1F
#define IS_SCR_MASK 0x20

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t *group;

/* Only EVER append values to this struct */
const jolt_gui_event_enum_t jolt_gui_event = {
        .pressed       = LV_EVENT_PRESSED,       /*The object has been pressed*/
        .pressing      = LV_EVENT_PRESSING,      /*The object is being pressed (called continuously while pressing)*/
        .press_list    = LV_EVENT_PRESS_LOST,    /*Still pressing but slid from the objects*/
        .short_clicked = LV_EVENT_SHORT_CLICKED, /*Released before long press time. Not called if dragged.*/
        .long_pressed =
                LV_EVENT_LONG_PRESSED, /*Pressing for `LV_INDEV_LONG_PRESS_TIME` time.  Not called if dragged.*/
        .long_pressed_repeat = LV_EVENT_LONG_PRESSED_REPEAT, /*Called after `LV_INDEV_LONG_PRESS_TIME` in every
                                         `LV_INDEV_LONG_PRESS_REP_TIME` ms.  Not called if dragged.*/
        .clicked          = LV_EVENT_CLICKED,  /*Called on release if not dragged (regardless to long press)*/
        .released         = LV_EVENT_RELEASED, /*Called in every cases when the object has been released*/
        .drag_begin       = LV_EVENT_DRAG_BEGIN,
        .drag_end         = LV_EVENT_DRAG_END,
        .drag_throw_begin = LV_EVENT_DRAG_THROW_BEGIN,
        .focused          = LV_EVENT_FOCUSED,
        .defocused        = LV_EVENT_DEFOCUSED,
        .value_changed    = LV_EVENT_VALUE_CHANGED,
        .insert           = LV_EVENT_INSERT,
        .refresh          = LV_EVENT_REFRESH,
        .apply            = LV_EVENT_APPLY,  /*"Ok", "Apply" or similar specific button has clicked*/
        .cancel           = LV_EVENT_CANCEL, /*"Close", "Cancel" or similar specific button has clicked*/
        .delete           = LV_EVENT_DELETE,
};

/**********************
 *      MACROS
 **********************/
#if PC_SIMULATOR
    #define MSG( ... ) printf( __VA_ARGS__ )
#elif ESP_PLATFORM
    #include "esp_log.h"
static const char TAG[] = "jolt_gui";
    #define MSG( ... ) ESP_LOGI( TAG, __VA_ARGS__ )
#else
    #define MSG( ... ) printf( __VA_ARGS__ )
#endif

/*********************
 * Screen Management *
 *********************/

jolt_gui_obj_t *jolt_gui_scr_get( const jolt_gui_obj_t *obj )
{
    const jolt_gui_obj_t *scr = obj;
    ESP_LOGD( TAG, "(%d) Getting screen of %p", __LINE__, obj );
    JOLT_GUI_CTX
    {
        if( NULL == obj ) {
            obj = BREAK_IF_NULL( lv_group_get_focused( group ) );
            ESP_LOGD( TAG, "(%d) Inferring currently focused object: %p", __LINE__, obj );
        }
        const lv_obj_t *tmp = obj;
        while( ( tmp = lv_obj_get_parent( tmp ) ) ) {
            if( tmp != lv_scr_act() ) { scr = tmp; }
        }
    }
    ESP_LOGD( TAG, "(%d) Screen of %p is %p", __LINE__, obj, scr );
    return (jolt_gui_obj_t *)scr;
}

lv_res_t jolt_gui_scr_del( jolt_gui_obj_t *obj )
{
    lv_res_t res           = LV_RES_OK;
    jolt_gui_obj_t *parent = NULL;
    JOLT_GUI_CTX
    {
        parent = BREAK_IF_NULL( jolt_gui_scr_get( obj ) );
        ESP_LOGD( TAG, "Deleting screen %p", parent );
        jolt_gui_obj_del( parent );
        res = LV_RES_INV;
    }
    return res;
}

/**
 * @brief Set the `is_scr` flag of a jolt gui object.
 * @param[in] obj
 * @param[in] val Value to set the flag to. true==is a screen, false==not a screen.
 */
static void jolt_gui_obj_id_is_scr_set( lv_obj_t *obj, bool val )
{
#if JOLT_GUI_USE_RESERVED
    if( val ) { obj->reserved = IS_SCR_MASK | ( ~IS_SCR_MASK & obj->reserved ); }
    else {
        obj->reserved = ~IS_SCR_MASK & obj->reserved;
    }
    ESP_LOGD( TAG, "set_scr: 0x%02X; reserved: 0x%02X", val, obj->reserved );
#else
    lv_obj_get_user_data_ptr( obj )->is_scr = val;
#endif
}

static bool jolt_gui_obj_id_is_scr_get( const lv_obj_t *obj )
{
#if JOLT_GUI_USE_RESERVED
    return IS_SCR_MASK & obj->reserved;
#else
    return lv_obj_get_user_data_ptr( (lv_obj_t *)obj )->is_scr;
#endif
}

void jolt_gui_obj_id_set( lv_obj_t *obj, jolt_gui_obj_id_t id )
{
#if JOLT_GUI_USE_RESERVED
    obj->reserved = ( id & ID_MASK ) | ( obj->reserved & ~ID_MASK );
    ESP_LOGV( TAG, "obj id: 0x%02X; reserved: 0x%02X", id, obj->reserved );
#else
    lv_obj_get_user_data_ptr( obj )->id = id;
#endif
    jolt_gui_obj_id_is_scr_set( obj, false );
}

jolt_gui_obj_id_t jolt_gui_obj_id_get( const lv_obj_t *obj )
{
    jolt_gui_obj_id_t id;
    if( jolt_gui_obj_id_is_scr_get( obj ) ) {
        ESP_LOGE( TAG, "Attempted to get object ID from screen" );
        return JOLT_GUI_OBJ_ID_INVALID;
    }
#if JOLT_GUI_USE_RESERVED
    id = obj->reserved & ID_MASK;
#else
    id                                  = lv_obj_get_user_data_ptr( (lv_obj_t *)obj )->id;
#endif
    return id;
}

void jolt_gui_scr_id_set( lv_obj_t *obj, jolt_gui_scr_id_t id )
{
#if JOLT_GUI_USE_RESERVED
    obj->reserved = ( id & ID_MASK ) | ( obj->reserved & ~ID_MASK );
    ESP_LOGD( TAG, "scr id: 0x%02X; reserved: 0x%02X", id, obj->reserved );
#else
    lv_obj_get_user_data_ptr( obj )->id = id;
#endif
    jolt_gui_obj_id_is_scr_set( obj, true );
}

jolt_gui_scr_id_t jolt_gui_scr_id_get( const lv_obj_t *obj )
{
    jolt_gui_scr_id_t id;
    if( !jolt_gui_obj_id_is_scr_get( obj ) ) {
        ESP_LOGE( TAG, "Attempted to get screen ID from an object" );
        return JOLT_GUI_SCR_ID_INVALID;
    }
#if JOLT_GUI_USE_RESERVED
    id = obj->reserved & ID_MASK;
#else
    id                                  = lv_obj_get_user_data_ptr( (lv_obj_t *)obj )->id;
#endif
    return id;
}

/**************************************
 * STANDARD SCREEN CREATION FUNCTIONS *
 **************************************/

/* Creates a dummy invisible object to anchor lvgl objects on the screen */
lv_obj_t *jolt_gui_obj_parent_create()
{
    lv_obj_t *parent = NULL;
    JOLT_GUI_CTX
    {
        parent = BREAK_IF_NULL( lv_obj_create( lv_scr_act(), NULL ) );
        ESP_LOGD( TAG, "Creating screen %p", parent );
        lv_obj_set_size( parent, LV_HOR_RES, LV_VER_RES );
        lv_obj_set_pos( parent, 0, 0 );
        lv_obj_set_style( parent, &lv_style_transp );
    }
    return parent;
}

/* Creates the statusbar title label for a screen. Returns the
 * label object. */
lv_obj_t *jolt_gui_obj_title_create( lv_obj_t *parent, const char *title )
{
    if( NULL == parent ) { return NULL; }

    lv_obj_t *label = NULL;
    JOLT_GUI_CTX
    {
        lv_coord_t title_cont_w = LV_HOR_RES_MAX;
        {
            lv_obj_t *statusbar_label = jolt_gui_statusbar_get_label();
            if( NULL != statusbar_label ) title_cont_w = lv_obj_get_x( statusbar_label ) - 1;
        }
        /* Create a non-transparent background to block out old titles */
        lv_obj_t *title_cont = BREAK_IF_NULL( lv_cont_create( parent, NULL ) );
        jolt_gui_obj_id_set( title_cont, JOLT_GUI_OBJ_ID_CONT_TITLE );
        lv_obj_align( title_cont, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0 );
        lv_obj_set_size( title_cont, title_cont_w, CONFIG_JOLT_GUI_STATUSBAR_H - 1 );

        label = BREAK_IF_NULL( lv_label_create( title_cont, NULL ) );
        jolt_gui_obj_id_set( label, JOLT_GUI_OBJ_ID_LABEL_0 );
        const lv_style_t *label_style = lv_obj_get_style( label );
        lv_label_set_long_mode( label, LV_LABEL_LONG_SROLL );
        lv_label_set_body_draw( label, false );  // dont draw background
        lv_obj_align( label, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0 );
        if( NULL == title ) { lv_label_set_text( label, EMPTY_STR ); }
        else {
            lv_label_set_text( label, title );
        }
        lv_obj_set_size( label, lv_obj_get_width( title_cont ), label_style->text.font->line_height );
    }
    return label;
}

/* Creates the body container */
lv_obj_t *jolt_gui_obj_cont_body_create( lv_obj_t *parent )
{
    if( NULL == parent ) { return NULL; }

    lv_obj_t *cont = NULL;
    JOLT_GUI_CTX
    {
        cont = BREAK_IF_NULL( lv_cont_create( parent, NULL ) );
        jolt_gui_obj_id_set( cont, JOLT_GUI_OBJ_ID_CONT_BODY );
        lv_obj_set_size( cont, LV_HOR_RES, LV_VER_RES - CONFIG_JOLT_GUI_STATUSBAR_H );
        lv_obj_align( cont, NULL, LV_ALIGN_IN_TOP_LEFT, 0, CONFIG_JOLT_GUI_STATUSBAR_H );
    }
    return cont;
}

void jolt_gui_obj_del( lv_obj_t *obj )
{
    if( NULL == obj ) { return; }

    ESP_LOGD( TAG, "Deleting obj %p", obj );
    JOLT_GUI_CTX { lv_obj_del( obj ); }
}

/***************
 * Group Stuff *
 ***************/
void jolt_gui_group_create()
{
    /* Create Groups for user input */
    bool success = false;
    JOLT_GUI_CTX
    {
        lv_theme_t *th = lv_theme_get_current();
        group          = BREAK_IF_NULL( lv_group_create() );
        lv_group_set_refocus_policy( group, LV_GROUP_REFOCUS_POLICY_PREV );
        lv_group_set_style_mod_cb( group, th->group.style_mod_xcb );
        lv_group_set_style_mod_edit_cb( group, th->group.style_mod_edit_xcb );
        success = true;
    }
    if( !success ) { esp_restart(); }
}

void jolt_gui_group_add( lv_obj_t *obj )
{
    JOLT_GUI_CTX
    {
        ESP_LOGD( TAG, "Adding %p to group", obj );
        lv_group_add_obj( group, obj );
        lv_group_focus_obj( obj );
    }
}

lv_group_t *jolt_gui_group_get() { return group; }

/**********
 * Action *
 **********/

lv_obj_t *jolt_gui_scr_get_active( const lv_obj_t *parent )
{
    lv_obj_t *obj = NULL;
    JOLT_GUI_CTX
    {
        parent = jolt_gui_scr_get( parent );
        /* Find the BODY CONT */
        lv_obj_t *cont_body = JOLT_GUI_FIND_AND_CHECK( parent, JOLT_GUI_OBJ_ID_CONT_BODY );

        /* Find the child thats in the group */
        lv_obj_t *child = NULL;
        while( NULL != ( child = lv_obj_get_child( cont_body, child ) ) ) {
            if( group == lv_obj_get_group( child ) ) {
                obj = child;
                break;
            }
        }
    }
    if( NULL == obj ) { ESP_LOGW( TAG, "Couldn't find screen's active object." ); }
    else {
        ESP_LOGD( TAG, "Active object: %p", obj );
    }
    return obj;
}

void jolt_gui_scr_set_event_cb( lv_obj_t *parent, lv_event_cb_t event_cb )
{
    JOLT_GUI_CTX
    {
        parent           = jolt_gui_scr_get( parent );
        lv_obj_t *active = jolt_gui_scr_get_active( parent );
        if( NULL != active ) { jolt_gui_obj_set_event_cb( active, event_cb ); }
    }
}

void jolt_gui_scr_set_active_param( lv_obj_t *parent, void *param )
{
    JOLT_GUI_CTX
    {
        parent           = jolt_gui_scr_get( parent );
        lv_obj_t *active = jolt_gui_scr_get_active( parent );
        if( NULL != active ) { jolt_gui_obj_set_param( active, param ); }
    }
}

void *jolt_gui_scr_get_active_param( const lv_obj_t *parent )
{
    void *param = NULL;
    JOLT_GUI_CTX
    {
        parent           = jolt_gui_scr_get( parent );
        lv_obj_t *active = jolt_gui_scr_get_active( parent );
        if( NULL != active ) { param = jolt_gui_obj_get_param( active ); }
    }
    return param;
}

void jolt_gui_event_del( lv_obj_t *obj, lv_event_t event )
{
    ESP_LOGD( TAG, "%s: (%d) event %s (%d)", __func__, __LINE__, jolt_gui_event_to_str( event ), event );
    if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( obj );
        ESP_LOGD( TAG, "Screen Deleted" );
    }
}

/********
 * MISC *
 ********/
static SemaphoreHandle_t jolt_gui_mutex = NULL;

void jolt_gui_sem_take()
{
    if( NULL == jolt_gui_mutex ) {
        /* Create the jolt_gui_mutex; avoids need to explicitly initialize */
        jolt_gui_mutex = xSemaphoreCreateRecursiveMutex();
        if( NULL == jolt_gui_mutex ) { esp_restart(); }
    }
    xSemaphoreTakeRecursive( jolt_gui_mutex, portMAX_DELAY );
}

void jolt_gui_sem_give() { xSemaphoreGiveRecursive( jolt_gui_mutex ); }

/* Finds the first child object with the object identifier.
 * Returns NULL if child not found. */
lv_obj_t *jolt_gui_find( const lv_obj_t *parent, jolt_gui_obj_id_t id )
{
    lv_obj_t *child = NULL;
    if( NULL == parent ) {
        ESP_LOGW( TAG, "Cannot search a NULL pointer." );
        return NULL;
    }
    JOLT_GUI_CTX
    {
        ESP_LOGD( TAG, "Searching the %d children of %p for %s", lv_obj_count_children( parent ), parent,
                  jolt_gui_obj_id_str( id ) );
        while( NULL != ( child = lv_obj_get_child( parent, child ) ) ) {
            jolt_gui_obj_id_t child_id;
            child_id = jolt_gui_obj_id_get( child );
            ESP_LOGV( TAG, "Child: %p %s", child, jolt_gui_obj_id_str( child_id ) );
            if( child_id == id ) { break; }
        }
    }
    if( NULL == child ) { ESP_LOGI( TAG, "Couldn't find a %s child.", jolt_gui_obj_id_str( id ) ); }
    return child;
}

/* Convert the enumerated value to a constant string */
const char *jolt_gui_obj_id_str( jolt_gui_obj_id_t val )
{
    const char *names[] = {FOREACH_JOLT_GUI_OBJ_ID( GENERATE_STRING )};
    if( val >= JOLT_GUI_OBJ_ID_MAX ) { return "<unknown>"; }
    return names[val];
}

/* Convert the enumerated value to a constant string */
const char *jolt_gui_scr_id_str( jolt_gui_scr_id_t val )
{
    const char *names[] = {FOREACH_JOLT_GUI_SCR_ID( GENERATE_STRING )};
    if( val >= JOLT_GUI_SCR_ID_MAX ) { return "<unknown>"; }
    return names[val];
}

static FILE *lvgl_in = NULL, *lvgl_out = NULL, *lvgl_err = NULL;

void jolt_gui_set_stdstream( FILE *in, FILE *out, FILE *err )
{
    lvgl_in  = in;
    lvgl_out = out;
    lvgl_err = err;
}

void jolt_gui_apply_stdstream()
{
    if( NULL != lvgl_in ) stdin = lvgl_in;
    if( NULL != lvgl_out ) stdout = lvgl_out;
    if( NULL != lvgl_err ) stderr = lvgl_err;
}

/************
 * Wrappers *
 ************/
lv_obj_t *jolt_gui_obj_get_parent( const lv_obj_t *obj ) { return lv_obj_get_parent( obj ); }

void jolt_gui_obj_set_event_cb( lv_obj_t *obj, lv_event_cb_t event_cb ) { lv_obj_set_event_cb( obj, event_cb ); }

void *jolt_gui_obj_get_param( const lv_obj_t *obj ) { return lv_obj_get_user_data_ptr( obj )->param; }

void jolt_gui_obj_set_param( lv_obj_t *obj, void *param ) { lv_obj_get_user_data_ptr( obj )->param = param; }

/*****************
 * Unity Asserts *
 *****************/
#if UNIT_TESTING
    #include "unity.h"

void TEST_ASSERT_EQUAL_DISPLAY( const jolt_display_t *expected, const jolt_display_t *actual )
{
    TEST_ASSERT_EQUAL_DISPLAY_MESSAGE( expected, actual, "Display buffer mismatch." );
}

void TEST_ASSERT_EQUAL_DISPLAY_MESSAGE( const jolt_display_t *expected, const jolt_display_t *actual, const char *msg )
{
    bool needs_free = false;
    jolt_display_t actual_proxy;

    if( NULL == expected ) {
        printf( "\nNo Expected data provided.\n" );
        goto dump;
    }
    else if( NULL == actual ) {
        /* Capture a screen and perform a comparison */
        actual_proxy.type     = expected->type;
        actual_proxy.encoding = JOLT_ENCODING_JRLE;
        actual                = &actual_proxy;
        needs_free            = true;
        TEST_ASSERT_TRUE_MESSAGE( jolt_display_copy( &actual_proxy ), "Failed to copy display buffer." );
    }

    if( expected->type != actual->type ) {
        printf( "\nDisplay type mismatch.\n" );
        goto dump;
    }

    /* Decode both buffers incase encoding doesn't match */
    int decode_len;
    uint8_t buf1[JOLT_DISPLAY_BUF_SIZE], buf2[JOLT_DISPLAY_BUF_SIZE];
    jolt_encoding_t decoder1, decoder2;
    decoder1   = jolt_encoding_get_decoder( expected->encoding );
    decoder2   = jolt_encoding_get_decoder( actual->encoding );
    decode_len = decoder1( buf1, sizeof( buf1 ), expected->data, expected->len );
    if( JOLT_DISPLAY_BUF_SIZE != decode_len ) {
        printf( "\nExpected Data didn't decode to correct length ( %d expected vs %d actual ).\n",
                JOLT_DISPLAY_BUF_SIZE, decode_len );
        goto dump;
    }
    decode_len = decoder2( buf2, sizeof( buf2 ), actual->data, actual->len );
    if( JOLT_DISPLAY_BUF_SIZE != decode_len ) {
        printf( "\nActual Data didn't decode to correct length ( %d expected vs %d actual ).\n", JOLT_DISPLAY_BUF_SIZE,
                decode_len );
        goto dump;
    }

    if( 0 != memcmp( buf1, buf2, JOLT_DISPLAY_BUF_SIZE ) ) {
        printf( "\nDisplay buffer mismatch.\n" );
        goto dump;
    }

    return;

dump:
    printf( "\nExpected:\n" );
    if( expected )
        jolt_display_print( expected );
    else
        printf( "NULL\n" );
    printf( "\nActual:\n" );
    if( actual )
        jolt_display_print( actual );
    else
        printf( "NULL\n" );
    printf( "\nDump actual:\n" );
    if( actual )
        jolt_display_dump( actual );
    else
        printf( "NULL\n" );
    printf( "\n" );

    if( needs_free ) jolt_display_free( &actual_proxy );
    if( msg )
        TEST_FAIL_MESSAGE( msg );
    else
        TEST_FAIL();
}

void TEST_ASSERT_BLANK_DISPLAY( const jolt_display_t *actual )
{
    TEST_ASSERT_BLANK_DISPLAY_MESSAGE( actual, "Dirty screen." );
}

void TEST_ASSERT_BLANK_DISPLAY_MESSAGE( const jolt_display_t *actual, const char *msg )
{
    uint8_t empty_display_data[] = {0x80, 0x00, 0x80, 0x08, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x03, 0x00};

    const jolt_display_t empty_display = {
            .type     = JOLT_DISPLAY_TYPE_SSD1306,
            .encoding = JOLT_ENCODING_RLE,
            .len      = sizeof( empty_display_data ),
            .data     = empty_display_data,
    };
    TEST_ASSERT_EQUAL_DISPLAY_MESSAGE( &empty_display, actual, msg );
}

#endif
