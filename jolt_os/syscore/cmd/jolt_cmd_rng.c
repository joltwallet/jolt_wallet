#include "jolt_helpers.h"
#include "sodium.h"
#include "stdio.h"
#include "syscore/cli_helpers.h"
#include "syscore/filesystem.h"

int jolt_cmd_rng( int argc, char** argv )
{
    int return_code    = -1;
    uint8_t bin_buf[16] = {0};
    bool output_hex    = false;
    uint64_t n_bytes   = 0;

    if( !console_check_range_argc( argc, 2, 3 ) ) {
        printf( "Specify how many bytes to generate.\n" );
        return_code = -1;
        goto exit;
    }

    for( uint8_t i = 1; i < argc; i++ ) {
        if( 0 == strcmp( "--hex", argv[i] ) ) { output_hex = true; }
        else {
            n_bytes = strtoull( argv[i], NULL, 10 );
        }
    }

    jolt_suspend_logging();
    for( uint64_t i = 0; i < n_bytes; i += sizeof(bin_buf) ) {
        uint8_t gen_bytes = sizeof( bin_buf );
        if( n_bytes - i < sizeof(bin_buf) ) { gen_bytes = n_bytes - i; }
        jolt_random( bin_buf, gen_bytes );
        if( output_hex ) {
            char hex_buf[2*sizeof(bin_buf)+1];
            sodium_bin2hex( hex_buf, sizeof( hex_buf ), bin_buf, gen_bytes );

            fwrite( hex_buf, 1, gen_bytes, stdout );
        }
        else {
            fwrite( bin_buf, 1, gen_bytes, stdout );
        }
    }
    printf( "\n" );
    jolt_resume_logging();

    return_code = 0;

exit:
    return return_code;
}
