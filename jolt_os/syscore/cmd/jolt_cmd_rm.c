#include "stdio.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "syscore/cli_helpers.h"
#include "syscore/filesystem.h"
#include "jolt_helpers.h"


int jolt_cmd_rm(int argc, char** argv) {
    int return_code = -1;

    if( !console_check_range_argc(argc, 2, 32) ) {
        return_code = 1;
        goto exit;
    }

    for(uint8_t i=1; i<argc; i++){
        char fn[128]=SPIFFS_BASE_PATH;
        strcat(fn, "/");
        strncat(fn, argv[i], sizeof(fn)-strlen(fn)-1);
        if( jolt_fs_exists(fn) ) {
            remove(fn);
            return_code = 0;
            jolt_h_fn_home_refresh(fn);
        }
        else {
            printf("File %s doesn't exist!\n", fn);
            return_code = 1;
        }
    }
exit:
    return return_code;
}


