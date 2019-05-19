#include "stdio.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "syscore/cli_helpers.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char TAG[] = "cmd_download";


int jolt_cmd_download(int argc, char** argv) {
    int return_code;
    if( !console_check_equal_argc(argc, 2) ) {
        return_code = 1;
        goto exit;
    }

    char fname[128] = SPIFFS_BASE_PATH;
    strcat(fname, "/");
    strncat(fname, argv[1], sizeof(fname)-strlen(fname)-1);

    FILE *ffd = fopen(fname, "rb");
    int trans_res=-1;
    if (ffd) {
        size_t fsize = jolt_fs_size(fname);
        printf("Receive file on host over YModem using a command like:\n");
        printf("rz --ymodem > /dev/ttyUSB0 < /dev/ttyUSB0\n");
        printf("\r\nReady to send %d byte file \"%s\", please start YModem receive on host ...\r\n", fsize, fname);
        trans_res = ymodem_transmit(argv[1], fsize, ffd);
        fclose(ffd);
        printf("\r\n");
        if (trans_res == 0) {
            ESP_LOGI(TAG, "Transfer complete.");
            return_code = 0;
        }
        else {
            ESP_LOGE(TAG, "Transfer failed, Error=%d", trans_res);
            return_code = trans_res;
            goto exit;
        }
    }
    else {
        ESP_LOGE(TAG, "Error opening file \"%s\" for sending.", fname);
        return_code = 3;
        goto exit;
    }

exit:
    return return_code;
}

