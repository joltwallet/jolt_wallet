#include "stdio.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "syscore/console_helpers.h"
#include "syscore/filesystem.h"
#include "jolt_helpers.h"


int jolt_cmd_mv(int argc, char** argv) {
    int return_code;
    if( !console_check_equal_argc(argc, 3) ) {
        return_code = 1;
        goto exit;
    }
    char src_fn[128] = SPIFFS_BASE_PATH;
    strcat(src_fn, "/");
    strncat(src_fn, argv[1], sizeof(src_fn)-strlen(src_fn)-1);

    char dst_fn[128] = SPIFFS_BASE_PATH;
    strcat(dst_fn, "/");
    strncat(dst_fn, argv[2], sizeof(dst_fn)-strlen(dst_fn)-1);

    return_code = rename(src_fn, dst_fn);

    if( 0 == return_code ) {
        jolt_h_fn_home_refresh( argv[2] );
    }

exit:
    return return_code;
}


