#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "jolt_helpers.h"
#include "stdio.h"
#include "syscore/cli_helpers.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"

int jolt_cmd_download( int argc, char **argv )
{
    int return_code;
    char *fname = NULL;
    FILE *ffd   = NULL;

    if( !console_check_equal_argc( argc, 2 ) ) EXIT( -1 );

    if( NULL == ( fname = jolt_fs_parse( argv[1], NULL ) ) ) EXIT( -2 );

    if( NULL == ( ffd = fopen( fname, "rb" ) ) ) EXIT_PRINT( -3, "Error opening file \"%s\".\n", fname );

    {
        size_t fsize = jolt_fs_size( fname );
        if( 0 == ymodem_transmit( argv[1], fsize, ffd ) )
            EXIT( 0 );
        else
            EXIT_PRINT( -4, "Transfer failedi.\n" );
    }

    EXIT( 0 );

exit:
    SAFE_FREE( fname );
    SAFE_CLOSE( ffd );
    return return_code;
}
