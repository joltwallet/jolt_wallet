#include "jolt_helpers.h"
#include "stdio.h"
#include "syscore/cli_helpers.h"
#include "stdlib.h"

int jolt_cmd_long( int argc, char** argv )
{
    const char ascii[] = "0123456789ABCDEF";
    int return_code = -1;
    if( !console_check_equal_argc( argc, 2 ) ) EXIT_PRINT( -1, "Specify only a integer bytelength.\n" );

    int n_bytes = atoi(argv[1]);

    for(int32_t i=0; i < n_bytes; i++ ){
        uint8_t index = i % 16;
        printf("%c", ascii[index]);
    }
    printf("\n");
    return_code = 0;
exit:
    return return_code;
}
