#include "esp_vfs.h"
#include "jolt_helpers.h"
#include "stdio.h"
#include "syscore/cli_helpers.h"
#include "syscore/filesystem.h"

int jolt_cmd_cat( int argc, char **argv )
{
    int return_code = -1;
    FILE *f         = NULL;
    char *fn        = NULL;
    char c;

    if( !console_check_equal_argc( argc, 2 ) ) EXIT_PRINT( -1, "Specify only a single file.\n" );

    fn = jolt_fs_parse( argv[1], NULL );
    if( NULL == fn ) EXIT_PRINT( -2, "Invalid filename\n" );

    if( !jolt_fs_exists( fn ) ) EXIT_PRINT( -3, "File doesn't exist!\n" );

    f = fopen( fn, "r" );
    if( NULL == f ) EXIT_PRINT( -4, "Error opening file." );

    /* Print all characters in the file */
    while( 0 != fread( &c, 1, 1, f ) ) { printf( "%c", c ); }
    printf( "\n" );

    EXIT( 0 );

exit:
    SAFE_CLOSE( f );
    SAFE_FREE( fn );
    return return_code;
}
