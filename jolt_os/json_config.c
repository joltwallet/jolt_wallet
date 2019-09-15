#include "json_config.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "jolt_helpers.h"
#include "syscore/filesystem.h"
#include "syscore/launcher.h"

static const char TAG[] = "json_config";
static const char EXT[] = "json";

cJSON *jolt_json_read( const char *fn )
{
    FILE *f      = NULL;
    cJSON *json  = NULL;
    size_t fsize = 0;
    char *buf    = NULL;

    /* Make sure the fn ends in ".json" */
    if( !jolt_strcmp_suffix( fn, ".json" ) ) {
        ESP_LOGE( TAG, "file %s does not end in \".json\"", fn );
        goto exit;
    }

    /* Make sure the file exists */
    if( !jolt_fs_exists( fn ) ) {
        ESP_LOGE( TAG, "Executable doesn't exist\n" );
        goto exit;
    }

    /* Open the file */
    f = fopen( fn, "r" );
    if( f == NULL ) {
        ESP_LOGE( TAG, "Failed to open file %s for reading", fn );
        goto exit;
    }

    /* Get the size of the file */
    fseek( f, 0, SEEK_END );
    fsize = ftell( f );
    fseek( f, 0, SEEK_SET );

    /* Allocate the string buffer */
    buf = malloc( fsize + 1 ); /* 1 for NULL terminator */
    if( NULL == buf ) {
        ESP_LOGE( TAG, "Unable to allocate space for file contents." );
        goto exit;
    }

    /* Read in the entire file */
    {
        int len = fread( buf, fsize, 1, f );
        if( 1 != len ) {
            ESP_LOGE( TAG, "Unable to read file into memory" );
            goto exit;
        }
        buf[fsize] = '\0';
    }

    /* Close the file */
    fclose( f );
    f = NULL;

    /* Parse the string */
    json = cJSON_Parse( buf );
    if( NULL == json ) {
        ESP_LOGE( TAG, "Error parsing JSON data:\n%s", buf );
        goto exit;
    }

    /* Free the buffer */
    free( buf );

    return json;

exit:
    if( NULL != json ) { cJSON_Delete( json ); }
    if( NULL != buf ) { free( buf ); }
    if( NULL != f ) { fclose( f ); }
    return NULL;
}

cJSON *jolt_json_read_app()
{
    char *name = NULL, *path = NULL;
    cJSON *json = NULL;
    if( NULL == ( name = launch_get_name() ) ) goto exit;
    if( NULL == ( path = jolt_fs_parse( name, EXT ) ) ) goto exit;
    json = jolt_json_read( path );

exit:
    SAFE_FREE( path );
    return json;
}

int jolt_json_write( const char *fn, const cJSON *json )
{
    FILE *f   = NULL;
    char *str = NULL;

    /* Open the file */
    f = fopen( fn, "w" );
    if( f == NULL ) {
        ESP_LOGE( TAG, "Failed to open file %s for writing", fn );
        goto exit;
    }

    /* Convert JSON object into string */
    str = cJSON_Print( json );
    if( NULL == str ) {
        ESP_LOGE( TAG, "Failed to allocate space for JSON string" );
        goto exit;
    }
    ESP_LOGI( TAG, "JSON String (len %d): %s", strlen( str ), str );

    /* Write string to file */
    {
        size_t written;
        size_t len = strlen( str ) + 1;
        written    = fwrite( str, len, 1, f );
        if( 1 != written ) {
            ESP_LOGE( TAG, "Unable to write to file" );
            goto exit;
        }
    }

    fclose( f );
    f = NULL;

    return 0; /* Success */

exit:
    if( NULL != f ) { fclose( f ); }
    if( NULL != str ) { free( str ); }

    return -1; /* Failure */
}

int jolt_json_write_app( const cJSON *json )
{
    char *name = NULL, *path = NULL;
    int return_code;
    if( NULL == ( name = launch_get_name() ) ) EXIT( -1 );
    if( NULL == ( path = jolt_fs_parse( name, EXT ) ) ) EXIT( -2 );
    return_code = jolt_json_write( path, json );

exit:
    SAFE_FREE( path );
    return return_code;
}
