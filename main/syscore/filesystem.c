/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "menu8g2.h"
#include <libwebsockets.h>
#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include "nano_lws.h"
#include "nano_parse.h"

#include "esp_spiffs.h"
#include "ymodem.h"

#include "../globals.h"
#include "../console.h"
#include "../vault.h"
#include "../helpers.h"
#include "../gui/gui.h"
#include "../gui/loading.h"
#include "../gui/statusbar.h"
#include "../gui/confirmation.h"

#include "../console.h"

static const char* TAG = "console_syscore_fs";

#define MAX_FILE_SIZE (1048576 - 0x2000)
#define SPIFFS_BASE_PATH "/spiffs"


void filesystem_init() {
    esp_err_t ret;
    esp_vfs_spiffs_conf_t conf = {
      .base_path = SPIFFS_BASE_PATH,
      .partition_label = NULL,
      .max_files = 3,
      .format_if_mount_failed = true
    };
    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

static int check_file_exists(char *fname) {
    if (!esp_spiffs_mounted( NULL )) {
        return -1;
    }

    struct stat sb;
    if (stat(fname, &sb) == 0) {
        return 1;
    }
    return 0;
}

static size_t get_file_size(char *fname) {
    if (!esp_spiffs_mounted( NULL )) {
        return -1;
    }

    struct stat sb;
    if (stat(fname, &sb) == 0) {
        return sb.st_size;
    }
    else{
        return -1;
    }
}


static uint32_t fs_free() {
    uint32_t tot, used;
    esp_err_t ret = esp_spiffs_info(NULL, &tot, &used);
    return (tot-used-16384);
}

static int file_upload(int argc, char** argv) {

    FILE *ffd = NULL;
    int rec_res = -1;
    char tmp_fname[128] = SPIFFS_BASE_PATH;
    int32_t max_fsize = fs_free();

    // Open the file
    strcat(tmp_fname, "/tmp");
    if( check_file_exists(tmp_fname) ) {
        remove(tmp_fname);
    }
    ffd = fopen(tmp_fname, "wb");
    if (ffd) {
        printf("Send file from host over YModem using a command like:\n");
        printf("sz --ymodem cat.jpg > /dev/ttyUSB0 < /dev/ttyUSB0\n");
        printf("\r\nReady to receive file, please start YModem transfer on host ...\r\n");
        char orig_name[128] = SPIFFS_BASE_PATH;
        strcat(orig_name, "/");
        rec_res = Ymodem_Receive(ffd, max_fsize, orig_name + strlen(orig_name));
        fclose(ffd);
        printf("\r\n");
        if (rec_res > 0) {
            printf("\"%s\" Transfer complete, Size=%d Bytes\n",
                    orig_name+strlen(SPIFFS_BASE_PATH), rec_res);
            rename(tmp_fname, orig_name);
        }
        else {
            printf(TAG, "Transfer complete, Error=%d", rec_res);
            remove(tmp_fname);
        }
    }
    else {
        ESP_LOGE(TAG, "Error opening file \"%s\" for receive.", tmp_fname);
    }
    return 0;
}

static int file_download(int argc, char** argv) {
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
        size_t fsize = get_file_size(fname);
        printf("Receive file on host over YModem using a command like:\n");
        printf("rz --ymodem > /dev/ttyUSB0 < /dev/ttyUSB0\n");
        printf("\r\nReady to send %d byte file \"%s\", please start YModem receive on host ...\r\n", fsize, fname);
        trans_res = Ymodem_Transmit(argv[1], fsize, ffd);
        fclose(ffd);
        printf("\r\n");
        if (trans_res == 0) {
            ESP_LOGI(TAG, "Transfer complete.");
            return_code = 0;
        }
        else {
            ESP_LOGE(TAG, "Transfer complete, Error=%d", trans_res);
            return_code = 2;
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

static int file_mv(int argc, char** argv) {
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

exit:
    return return_code;
}

static int file_rm(int argc, char** argv) {
    int return_code;

    if( !console_check_range_argc(argc, 2, 32) ) {
        return_code = 1;
        goto exit;
    }

    for(uint8_t i=1; i<argc; i++){
        char fn[128]=SPIFFS_BASE_PATH;
        strcat(fn, "/");
        strncat(fn, argv[i], sizeof(fn)-strlen(fn)-1);
        if( check_file_exists(fn) ) {
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

static int file_ls(int argc, char** argv) {
    const char path[] = SPIFFS_BASE_PATH;

    DIR *dir = NULL;
    struct dirent *ent;
    char type;
    char size[9];
    char tpath[255];
    char tbuffer[80];
    struct stat sb;
    struct tm *tm_info;
    int statok;

    printf("LIST of DIR [%s]\r\n", path);
    // Open directory
    dir = opendir(path);
    if (!dir) {
        printf("Error opening directory\r\n");
        return  1;
    }

    // Read directory entries
    uint64_t total = 0;
    int nfiles = 0;
    printf("T    Size      Date/Time         Name\r\n");
    printf("-----------------------------------\r\n");
    while ((ent = readdir(dir)) != NULL) {
        sprintf(tpath, path);
        if (path[strlen(path)-1] != '/') strcat(tpath,"/");
        strcat(tpath,ent->d_name);
        tbuffer[0] = '\0';

        // Get file stat
        statok = stat(tpath, &sb);

        if (statok == 0) {
            tm_info = localtime(&sb.st_mtime);
            strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
        }
        else sprintf(tbuffer, "                ");

        if (ent->d_type == DT_REG) {
            type = 'f';
            nfiles++;
            if (statok) strcpy(size, "       ?");
            else {
                total += sb.st_size;
                if (sb.st_size < (1024*1024)) sprintf(size,"%8d", (int)sb.st_size);
                else if ((sb.st_size/1024) < (1024*1024)) sprintf(size,"%6dKB", (int)(sb.st_size / 1024));
                else sprintf(size,"%6dMB", (int)(sb.st_size / (1024 * 1024)));
            }
        }
        else {
            type = 'd';
            strcpy(size, "       -");
        }

        printf("%c  %s  %s  %s\r\n",
            type,
            size,
            tbuffer,
            ent->d_name
        );
    }
    if (total) {
        printf("-----------------------------------\r\n");
        if (total < (1024*1024)) printf("   %8d", (int)total);
        else if ((total/1024) < (1024*1024)) printf("   %6dKB", (int)(total / 1024));
        else printf("   %6dMB", (int)(total / (1024 * 1024)));
        printf(" in %d file(s)\r\n", nfiles);
    }
    printf("-----------------------------------\r\n");

    closedir(dir);

    uint32_t tot, used;
    esp_spiffs_info(NULL, &tot, &used);
    printf("SPIFFS: free %d KB of %d KB\r\n", (tot-used) / 1024, tot / 1024);
    return 0;
}


void console_syscore_fs_register() {
    esp_console_cmd_t cmd;

    cmd = (esp_console_cmd_t) {
        .command = "upload",
        .help = "Enters file UART ymodem upload mode",
        .hint = NULL,
        .func = &file_upload,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "download",
        .help = "Transmit specified file over UART ymodem",
        .hint = NULL,
        .func = &file_download,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "ls",
        .help = "List filesystem",
        .hint = NULL,
        .func = &file_ls,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "mv",
        .help = "rename file (src, dst)",
        .hint = NULL,
        .func = &file_mv,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "rm",
        .help = "remove file from filesystem",
        .hint = NULL,
        .func = &file_rm,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

}
