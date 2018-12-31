#include "stdio.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"

static const char TAG[] = "cmd_upload";

int jolt_cmd_upload(int argc, char** argv) {

    FILE *ffd = NULL;
    int rec_res = -1;
    int32_t max_fsize = jolt_fs_free();

    /* Open the file */
    char tmp_fn[CONFIG_SPIFFS_OBJ_NAME_LEN] = SPIFFS_BASE_PATH;
    char orig_fn[CONFIG_SPIFFS_OBJ_NAME_LEN] = SPIFFS_BASE_PATH;

    strcat(tmp_fn, "/tmp");
    if( jolt_fs_exists(tmp_fn) ) {
        remove(tmp_fn);
    }
    ffd = fopen(tmp_fn, "wb");
    if (ffd) {
        printf("Send file from host over YModem using a command like:\n"
                "sz --ymodem cat.jpg > /dev/ttyUSB0 < /dev/ttyUSB0\n"
                "Ready to receive file, please start YModem transfer on host ...\n");
        strcat(orig_fn, "/");
        rec_res = Ymodem_Receive(ffd, max_fsize, orig_fn + strlen(orig_fn), NULL);
        fclose(ffd);
        printf("\r\n");
        if (rec_res > 0) {
            printf("\"%s\" Transfer complete, Size=%d Bytes\n",
                    orig_fn+strlen(SPIFFS_BASE_PATH), rec_res);
            if( jolt_fs_exists(orig_fn) ) {
                remove(orig_fn);
            }
            rename(tmp_fn, orig_fn);
        }
        else {
            ESP_LOGE(TAG, "Transfer complete, Error=%d", rec_res);
            remove(tmp_fn);
        }
    }
    else {
        ESP_LOGE(TAG, "Error opening file \"%s\" for receive.", tmp_fn);
    }
    if( jolt_fs_exists(tmp_fn) ) {
        remove(tmp_fn);
    }

    return 0;
}


