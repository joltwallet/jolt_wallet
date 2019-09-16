//#define LOG_LOCAL_LEVEL 4

#include "cJSON.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "jolt_helpers.h"
#include "stdio.h"
#include "syscore/filesystem.h"

static const char TAG[] = "jolt_cmd_ls";

int jolt_cmd_ls( int argc, char **argv )
{
    int return_code = -1;
    DIR *dir        = NULL;
    struct dirent *ent;
    struct stat sb;
    cJSON *json = NULL, *json_files = NULL, *entry = NULL;
    char *response  = NULL;
    uint32_t nfiles = 0;

    if( NULL == ( json = cJSON_CreateObject() ) ) EXIT( -1 );
    if( NULL == ( json_files = cJSON_AddArrayToObject( json, "files" ) ) ) EXIT( -1 );

    if( !( dir = opendir( JOLT_FS_MOUNTPT ) ) ) EXIT_PRINT( -1, "Error opening directory\n" );

    /* Read directory entries */
    while( ( ent = readdir( dir ) ) != NULL ) {
        char tpath[JOLT_FS_MAX_ABS_PATH_BUF_LEN] = JOLT_FS_MOUNTPT;

        nfiles++;
        assert( ent->d_type == DT_REG ); /* All objects should be files */

        if( NULL == ( entry = cJSON_CreateObject() ) ) EXIT( -2 );
        if( NULL == cJSON_AddStringToObject( entry, "name", ent->d_name ) ) EXIT( -2 );

        /* Parse full filepath */
        if( tpath[strlen( tpath ) - 1] != '/' ) strlcat( tpath, "/", sizeof( tpath ) );
        strlcat( tpath, ent->d_name, sizeof( tpath ) );
        ESP_LOGD( TAG, "stat path \"%s\"", tpath );

        if( stat( tpath, &sb ) ) {
            /* Unknown size */
            if( NULL == cJSON_AddNumberToObject( entry, "size", -1 ) ) EXIT( -3 );
        }
        else {
            if( NULL == cJSON_AddNumberToObject( entry, "size", sb.st_size ) ) EXIT( -3 );
        }
        cJSON_AddItemToArray( json_files, entry );
        entry = NULL;
    }
    if( NULL == cJSON_AddNumberToObject( json, "n", nfiles ) ) EXIT( -3 );

    {
        uint32_t tot, used;
        jolt_fs_info( &tot, &used );
        if( NULL == cJSON_AddNumberToObject( json, "total", tot ) ) EXIT( -3 );
        if( NULL == cJSON_AddNumberToObject( json, "free", tot - used ) ) EXIT( -3 );
    }

    response = cJSON_Print( json );
    printf( response );
    printf( "\n" );

    EXIT( 0 );

exit:
    if( NULL != dir ) closedir( dir );
    if( NULL != json ) cJSON_Delete( json );
    if( NULL != entry ) cJSON_Delete( entry );
    if( NULL != response ) free( response );

    return return_code;
}
