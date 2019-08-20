#include "stdio.h"
#include "esp_vfs_dev.h"
#include "syscore/cli_helpers.h"
#include "syscore/filesystem.h"
#include "jolt_helpers.h"


int jolt_cmd_mv(int argc, char** argv) {
    int return_code;
    char *src_fn = NULL;
    char *dst_fn = NULL;

    /* Input Argument Check */
    if( !console_check_equal_argc(argc, 3) ) EXIT_PRINT(-1, "Requires 3 args: mv [src] [dst]");

    if( NULL == (src_fn = jolt_fs_parse(argv[1], NULL))) EXIT_PRINT(-2, "Invalid [src] filename");
    if( NULL == (dst_fn = jolt_fs_parse(argv[2], NULL))) EXIT_PRINT(-3, "Invalid [dst] filename");

    if( ESP_OK != jolt_fs_mv(src_fn, dst_fn) ) EXIT_PRINT(-4, "Unsuccessful move");

    EXIT(0); /* Success */

exit:
    SAFE_FREE(src_fn);
    SAFE_FREE(dst_fn);
    return return_code;
}


