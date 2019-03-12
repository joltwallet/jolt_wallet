#include "stdio.h"
#include "esp_vfs.h"
#include "syscore/filesystem.h"
#include "syscore/console_helpers.h"

int jolt_cmd_cat(int argc, char** argv) {
    int return_code = -1;
    FILE *f = NULL;
    char fn[128]=SPIFFS_BASE_PATH;
    char c;

    if( !console_check_equal_argc(argc, 2) ) {
        printf("Specify only a single file.\n");
        return_code = -1;
        goto exit;
    }

    strcat(fn, "/");
    strncat(fn, argv[1], sizeof(fn)-strlen(fn)-1);

    if( !jolt_fs_exists( fn ) ) {
        printf("File doesn't exist!\n");
        return_code = -2;
        goto exit;
    }

    f = fopen(fn, "r");
    if( NULL == f ) {
        return_code = -3;
        goto exit;
    }

    while( 0 != fread(&c, 1, 1, stdin) ) {
        printf("%c", c);
    }
    printf("\n");

    return_code = 0;

exit:
    if( NULL != f ) {
        fclose(f);
    }
    return return_code;
}
