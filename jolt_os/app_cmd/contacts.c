#include "contacts.h"
#include "jolt_gui/jolt_gui.h"
#include "json_config.h"
#include "syscore/cli_helpers.h"

#define CONFIG_JOLT_CONTACT_MAX      10
#define CONFIG_JOLT_CONTACT_NAME_LEN 128

#define CJSON_ERROR_CHECK( x )         \
    if( NULL == ( x ) ) {              \
        rc = JOLT_APP_CMD_CONTACT_OOM; \
        goto exit;                     \
    }

enum {
    JOLT_APP_CMD_CONTACT_ERROR = -1,
    JOLT_APP_CMD_CONTACT_OK    = 0,
    JOLT_APP_CMD_CONTACT_NOT_IMPLEMENTED,
    JOLT_APP_CMD_CONTACT_INVALID_ARGS,
    JOLT_APP_CMD_CONTACT_OOM,
};

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

static int contact_read( int argc, const char **argv )
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
    CJSON_ERROR_CHECK( buf = cJSON_PrintUnformatted( contacts ) );
    printf( buf );
    fflush( stdout );

    rc = JOLT_APP_CMD_CONTACT_OK;

exit:
    if( NULL != json ) cJSON_Delete( json );
    if( NULL != payload ) cJSON_Delete( json );
    if( NULL != buf ) free( buf );
    return rc;
}

static int contact_delete( int argc, const char **argv )
{
    int rc;
    int index;
    cJSON *json = NULL, *contacts;

    /* Argument Validation */
    {
        const char *index_str;
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
    CJSON_ERROR_CHECK( contacts = cJSON_DetachItemFromObject( json, "contacts" ) );

    {
        cJSON *e;
        CJSON_ERROR_CHECK( e = cJSON_GetArrayItem( contacts, index ) );
        cJSON_Delete( e );
    }

    /* Save */
    rc = jolt_json_write_app( json );

exit:
    if( NULL != json ) cJSON_Delete( json );
    return rc;
}

static int contact_add( int argc, const char **argv )
{
    int rc;
    cJSON *json    = NULL, *contacts;
    cJSON *contact = NULL;

    /* Argument Validation */
    const char *name, *address;
    if( !console_check_equal_argc( argc, 4 ) ) {
        rc = JOLT_APP_CMD_CONTACT_INVALID_ARGS;
        goto exit;
    }
    name    = argv[2];
    address = argv[3];

    /* Read in config and add entry */
    CJSON_ERROR_CHECK( json = read_config() );
    CJSON_ERROR_CHECK( contacts = cJSON_GetObjectItemCaseSensitive( json, "contacts" ) );
    CJSON_ERROR_CHECK( contact = cJSON_CreateObject() );
    cJSON_AddItemToArray( contacts, contact ); /* Add item first for easier cleanup on error */
    CJSON_ERROR_CHECK( cJSON_AddStringToObject( contact, "name", name ) );
    CJSON_ERROR_CHECK( cJSON_AddStringToObject( contact, "address", address ) );

    /* Save */
    rc = jolt_json_write_app( json );

exit:
    if( NULL != json ) cJSON_Delete( json );
    return rc;
}

static int contact_update( int argc, const char **argv )
{
    int rc;
    cJSON *json = NULL, *contacts;

    /* Argument Validation */
    int index;
    const char *index_str, *name, *address;
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

    /* Read in config */
    CJSON_ERROR_CHECK( json = read_config() );
    CJSON_ERROR_CHECK( contacts = cJSON_GetObjectItemCaseSensitive( json, "contacts" ) );

    cJSON *e;
    CJSON_ERROR_CHECK( e = cJSON_GetArrayItem( contacts, index ) );
    cJSON_DeleteItemFromObjectCaseSensitive( e, "name" );
    cJSON_DeleteItemFromObjectCaseSensitive( e, "address" );
    CJSON_ERROR_CHECK( cJSON_AddStringToObject( e, "name", name ) );
    CJSON_ERROR_CHECK( cJSON_AddStringToObject( e, "address", address ) );

    /* Save */
    rc = jolt_json_write_app( json );

exit:
    if( NULL != json ) cJSON_Delete( json );
    return rc;
}

static int contact_move( int argc, const char **argv )
{
    /* Argument Validation */
    const char *src_str, *dst_str;
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

static int contact_insert( int argc, const char **argv ) { return JOLT_APP_CMD_CONTACT_NOT_IMPLEMENTED; }

int jolt_app_cmd_contact( int argc, const char **argv )
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
    else if( 0 == strcmp( argv[1], "insert" ) ) {
        return contact_insert( argc, argv );
    }
    else if( 0 == strcmp( argv[1], "update" ) ) {
        return contact_update( argc, argv );
    }

    return JOLT_APP_CMD_CONTACT_NOT_IMPLEMENTED;
}
