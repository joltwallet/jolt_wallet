#include "stdio.h"
#include "syscore/filesystem.h"
#include "syscore/cli_helpers.h"
#include "jolt_helpers.h"
#include "sodium.h"

int jolt_cmd_rng(int argc, char** argv) {
    int return_code = -1;
    uint8_t bin_buf[4] = { 0 };
    bool output_hex = false;
    uint64_t n_bytes;

    if( !console_check_range_argc(argc, 2, 3) ) {
        printf("Specify how many bytes to generate.\n");
        return_code = -1;
        goto exit;
    }
    n_bytes = strtoull(argv[1], NULL, 10);

    if( argc == 3 && 0==strcmp("--hex", argv[2]) ) {
        output_hex = true;
    }

    for(uint64_t i=0; i < n_bytes; i+=4) {
        uint8_t gen_bytes = sizeof(bin_buf);
        if( n_bytes - i < 4) {
            gen_bytes = n_bytes - i;
        }
        jolt_get_random(bin_buf, gen_bytes);
        if( output_hex ){
            char hex_buf[9];
            sodium_bin2hex(hex_buf, sizeof(hex_buf), bin_buf, gen_bytes);

            fwrite(hex_buf, 1, gen_bytes, stdout);
        }
        else{
            fwrite(bin_buf, 1, gen_bytes, stdout);
        }
    }
    printf("\n");

    return_code = 0;

exit:
    return return_code;
}
