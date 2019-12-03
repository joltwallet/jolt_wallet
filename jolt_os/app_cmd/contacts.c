#include "contacts.h"
#include "errno.h"
#include "jolt_gui/jolt_gui.h"
#include "json_config.h"
#include "lang/lang.h"
#include "syscore/cli.h"
#include "syscore/cli_helpers.h"
#include "syscore/launcher.h"

#define CONFIG_JOLT_CONTACT_MAX         10
#define CONFIG_JOLT_CONTACT_NAME_LEN    128
#define CONFIG_JOLT_CONTACT_ADDRESS_LEN 192

#define CJSON_ERROR_CHECK( x )         \
    if( NULL == ( x ) ) {              \
        rc = JOLT_APP_CMD_CONTACT_OOM; \
        goto exit;                     \
    }

static const char TAG[] = "contacts";

/**
 * @brief callback for the confirmation screens.
 *
 * Saves the json stored in the gui active_parameter on ENTER
 * Doesn't save on BACK
 *
 * In either case, free the cJSON object and delete the screen
 */
static void confirmation_cb( jolt_gui_obj_t *obj, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        int rc;
        cJSON *json = jolt_gui_scr_get_active_param( obj );
        rc          = jolt_json_write_app( json );
        cJSON_Delete( json );
        jolt_gui_scr_del( obj );
        jolt_cli_return( rc );
    }
    else if( jolt_gui_event.cancel == event ) {
        cJSON *json = jolt_gui_scr_get_active_param( obj );
        cJSON_Delete( json );
        jolt_gui_scr_del( obj );
        jolt_cli_return( JOLT_APP_CMD_CONTACT_DECLINED );
    }
}

/**
 * @brief Read in json or create one if it doesn't exist.
 *
 * Makes sure it has a contacts array field.
 */
static cJSON *read_config()
{
    cJSON *json, *contacts;

    json = jolt_json_read_app();
    if( NULL == json ) {
        /* Create Default JSON */
        json = cJSON_CreateObject();
    }

    if( NULL == ( contacts = cJSON_GetObjectItemCaseSensitive( json, "contacts" ) ) ) {
        EXIT_IF_NULL( cJSON_AddArrayToObject( json, "contacts" ) );
    }
    else if( !cJSON_IsArray( contacts ) ) {
        cJSON_Delete( contacts );
        EXIT_IF_NULL( cJSON_AddArrayToObject( json, "contacts" ) );
    }

    return json;

exit:
    if( NULL != json ) cJSON_Delete( json );
    return NULL;
}

static int contact_read( int argc, char **argv )
{
    int rc;
    cJSON *json = NULL, *contacts, *payload = NULL;
    char *buf = NULL;

    /* Argument Validation */
    if( !console_check_equal_argc( argc, 2 ) ) return JOLT_APP_CMD_CONTACT_INVALID_ARGS;
    CJSON_ERROR_CHECK( json = read_config() );
    CJSON_ERROR_CHECK( payload = cJSON_CreateObject() );

    /* Isolate the contacts portion */
    CJSON_ERROR_CHECK( contacts = cJSON_DetachItemFromObject( json, "contacts" ) );
    cJSON_Delete( json );
    json = NULL;

    /* Recreate the overarching json object for printing */
    cJSON_AddItemToObjectCS( payload, "contacts", contacts );
    CJSON_ERROR_CHECK( buf = cJSON_PrintUnformatted( payload ) );
    printf( buf );
    fflush( stdout );

    rc = JOLT_APP_CMD_CONTACT_OK;

exit:
    if( NULL != json ) cJSON_Delete( json );
    if( NULL != payload ) cJSON_Delete( json );
    if( NULL != buf ) free( buf );
    return rc;
}

static int contact_delete( int argc, char **argv )
{
    int rc;
    int index;
    cJSON *json = NULL, *contacts;

    /* Argument Validation */
    {
        char *index_str;
        char *endptr;
        if( !console_check_equal_argc( argc, 3 ) ) {
            rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
            goto exit;
        }
        index_str = argv[2];
        errno     = 0;
        index     = strtol( index_str, &endptr, 10 );
        if( index_str == endptr || 0 != errno ) {
            rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
            goto exit;
        }
        if( index < 0 || index >= CONFIG_JOLT_CONTACT_MAX ) {
            rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
            goto exit;
        }
    }

    /* Read in config */
    CJSON_ERROR_CHECK( json = read_config() );
    CJSON_ERROR_CHECK( contacts = cJSON_GetObjectItemCaseSensitive( json, "contacts" ) );

    {
        char *name, *address;
        cJSON *e, *name_obj, *address_obj;
        CJSON_ERROR_CHECK( e = cJSON_DetachItemFromArray( contacts, index ) );
        CJSON_ERROR_CHECK( name_obj = cJSON_GetObjectItemCaseSensitive( e, "name" ) );
        CJSON_ERROR_CHECK( address_obj = cJSON_GetObjectItemCaseSensitive( e, "address" ) );
        name    = cJSON_GetStringValue( name_obj );
        address = cJSON_GetStringValue( address_obj );

        /* Create Confirmation Screen */
        char body_buf[128 + CONFIG_JOLT_CONTACT_NAME_LEN];
        char title_buf[140];
        snprintf( title_buf, sizeof( title_buf ), gettext( JOLT_TEXT_APP_CMD_CONTACT_TITLE ), jolt_launch_get_name() );
        snprintf( body_buf, sizeof( body_buf ), gettext( JOLT_TEXT_APP_CMD_CONTACT_DELETE ), name );
        jolt_gui_obj_t *scr = NULL;
        scr                 = jolt_gui_scr_text_create( title_buf, body_buf );
        jolt_gui_scr_scroll_add_monospace_text( scr, address );
        jolt_gui_scr_set_event_cb( scr, confirmation_cb );
        jolt_gui_scr_set_active_param( scr, json );

        cJSON_Delete( e );
    }

    return JOLT_CLI_NON_BLOCKING;

exit:
    if( NULL != json ) cJSON_Delete( json );
    return rc;
}

static int contact_add( int argc, char **argv )
{
    int rc;
    cJSON *json    = NULL, *contacts;
    cJSON *contact = NULL;

    /* Argument Validation */
    char *name, *address;
    if( !console_check_equal_argc( argc, 4 ) ) {
        rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
        goto exit;
    }
    name    = argv[2];
    address = argv[3];

    if( strlen( name ) >= CONFIG_JOLT_CONTACT_NAME_LEN ) {
        rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
        ESP_LOGE( TAG, "Contact name too long." );
        goto exit;
    }
    if( strlen( address ) >= CONFIG_JOLT_CONTACT_ADDRESS_LEN ) {
        rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
        ESP_LOGE( TAG, "Contact address too long." );
        goto exit;
    }

    /* Read in config and add entry */
    CJSON_ERROR_CHECK( json = read_config() );
    CJSON_ERROR_CHECK( contacts = cJSON_GetObjectItemCaseSensitive( json, "contacts" ) );
    CJSON_ERROR_CHECK( contact = cJSON_CreateObject() );
    cJSON_AddItemToArray( contacts, contact ); /* Add item first for easier cleanup on error */
    CJSON_ERROR_CHECK( cJSON_AddStringToObject( contact, "name", name ) );
    CJSON_ERROR_CHECK( cJSON_AddStringToObject( contact, "address", address ) );

    {
        /* Create Confirmation Screen */
        char body_buf[128 + CONFIG_JOLT_CONTACT_NAME_LEN];
        char title_buf[140];
        snprintf( title_buf, sizeof( title_buf ), gettext( JOLT_TEXT_APP_CMD_CONTACT_TITLE ), jolt_launch_get_name() );
        snprintf( body_buf, sizeof( body_buf ), gettext( JOLT_TEXT_APP_CMD_CONTACT_ADD ), name );
        jolt_gui_obj_t *scr = NULL;
        scr                 = jolt_gui_scr_text_create( title_buf, body_buf );
        jolt_gui_scr_scroll_add_monospace_text( scr, address );
        jolt_gui_scr_set_event_cb( scr, confirmation_cb );
        jolt_gui_scr_set_active_param( scr, json );
    }

    return JOLT_CLI_NON_BLOCKING;

exit:
    if( NULL != json ) cJSON_Delete( json );
    return rc;
}

static int contact_update( int argc, char **argv )
{
    int rc;
    cJSON *json = NULL, *contacts;

    /* Argument Validation */
    int index;
    char *index_str, *name, *address;
    if( !console_check_equal_argc( argc, 5 ) ) {
        rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
        goto exit;
    }
    index_str = argv[2];
    name      = argv[3];
    address   = argv[4];

    {
        char *endptr;
        errno = 0;
        index = strtol( index_str, &endptr, 10 );
        if( index_str == endptr || 0 != errno ) {
            rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
            goto exit;
        }
        if( index < 0 || index >= CONFIG_JOLT_CONTACT_MAX ) {
            rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
            goto exit;
        }
    }

    if( strlen( name ) >= CONFIG_JOLT_CONTACT_NAME_LEN ) {
        rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
        ESP_LOGE( TAG, "Contact name too long." );
        goto exit;
    }
    if( strlen( address ) >= CONFIG_JOLT_CONTACT_ADDRESS_LEN ) {
        rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
        ESP_LOGE( TAG, "Contact address too long." );
        goto exit;
    }

    /* Read in config */
    CJSON_ERROR_CHECK( json = read_config() );
    CJSON_ERROR_CHECK( contacts = cJSON_GetObjectItemCaseSensitive( json, "contacts" ) );

    {
        char *old_name, *old_address;
        cJSON *e, *name_obj, *address_obj;
        CJSON_ERROR_CHECK( e = cJSON_GetArrayItem( contacts, index ) );
        CJSON_ERROR_CHECK( name_obj = cJSON_DetachItemFromObjectCaseSensitive( e, "name" ) );
        CJSON_ERROR_CHECK( address_obj = cJSON_DetachItemFromObjectCaseSensitive( e, "address" ) );
        old_name    = cJSON_GetStringValue( name_obj );
        old_address = cJSON_GetStringValue( address_obj );
        CJSON_ERROR_CHECK( cJSON_AddStringToObject( e, "name", name ) );
        CJSON_ERROR_CHECK( cJSON_AddStringToObject( e, "address", address ) );

        /* Create Confirmation Screen */
        char body_buf[128 + 2 * CONFIG_JOLT_CONTACT_NAME_LEN];
        char title_buf[140];
        snprintf( title_buf, sizeof( title_buf ), gettext( JOLT_TEXT_APP_CMD_CONTACT_TITLE ), jolt_launch_get_name() );
        snprintf( body_buf, sizeof( body_buf ), gettext( JOLT_TEXT_APP_CMD_CONTACT_UPDATE ), old_name, name );
        jolt_gui_obj_t *scr = NULL;
        scr                 = jolt_gui_scr_text_create( title_buf, body_buf );
        jolt_gui_scr_scroll_add_monospace_text( scr, address );
        jolt_gui_scr_set_event_cb( scr, confirmation_cb );
        jolt_gui_scr_set_active_param( scr, json );

        cJSON_Delete( name_obj );
        cJSON_Delete( address_obj );
    }

    return JOLT_CLI_NON_BLOCKING;

exit:
    if( NULL != json ) cJSON_Delete( json );
    return rc;
}

#if 0
static int contact_move( int argc, char **argv )
{
    /* Argument Validation */
    char *src_str, *dst_str;
    int src, dst;
    if( !console_check_equal_argc( argc, 4 ) ) return JOLT_APP_CMD_CONTACT_INVALID_ARGS;
    src_str = argv[2];
    dst_str = argv[3];

    /* Read in config */
    int rc;
    cJSON *json, *contacts;
    CJSON_ERROR_CHECK( json = read_config() );
    CJSON_ERROR_CHECK( contacts = cJSON_GetObjectItemCaseSensitive( json, "contacts" ) );

    rc = JOLT_APP_CMD_CONTACT_NOT_IMPLEMENTED;

exit:
    if( NULL != json ) cJSON_Delete( json );
    return rc;
}
#endif

int jolt_app_cmd_contact( int argc, char **argv )
{
    /* Argument Verification */
    if( !console_check_range_argc( argc, 2, 5 ) ) { return 1; }

    /* More specific argument verification */
    if( 0 == strcmp( argv[1], "read" ) ) { return contact_read( argc, argv ); }
    else if( 0 == strcmp( argv[1], "delete" ) ) {
        return contact_delete( argc, argv );
    }
    else if( 0 == strcmp( argv[1], "add" ) ) {
        return contact_add( argc, argv );
    }
    else if( 0 == strcmp( argv[1], "update" ) ) {
        return contact_update( argc, argv );
    }

    return JOLT_APP_CMD_CONTACT_INVALID_ARGS;
}
