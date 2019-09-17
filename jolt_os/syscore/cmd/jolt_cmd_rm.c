#include "esp_vfs_dev.h"
#include "jolt_helpers.h"
#include "stdio.h"
#include "syscore/cli_helpers.h"
#include "syscore/filesystem.h"

int jolt_cmd_rm( int argc, char** argv )
{
    int return_code = -1;

    if( !console_check_range_argc( argc, 2, 32 ) ) EXIT( -1 );

    for( uint8_t i = 1; i < argc; i++ ) {
        char* fn;
        fn = jolt_fs_parse( argv[i], NULL );
        if( NULL == fn ) continue;

        if( jolt_fs_exists( fn ) ) {
            remove( fn );
            return_code = 0;
            jolt_fn_home_refresh( fn );
        }
        else {
            printf( "File %s doesn't exist!\n", fn );
            return_code = -2;
        }
        free( fn );
    }

exit:
    return return_code;
}
