#include "stdio.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "syscore/console_helpers.h"
#include "syscore/filesystem.h"


int jolt_cmd_rm(int argc, char** argv) {
    int return_code;

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
        }
        else {
            printf("File %s doesn't exist!\n", fn);
        }
    }
    return_code = 0;
exit:
    return return_code;
}


