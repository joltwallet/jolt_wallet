/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "sdkconfig.h"
#include "sodium.h"

#include "../console.h"
#include "jolt_gui/confirmation.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "vault.h"
#include "filesystem.h"
#include "decompress.h"
#include "syscore/console_helpers.h"

static const char* TAG = "console_syscore_fs";

/* Starts up the SPIFFS Filesystem */
void jolt_fs_init() {
    esp_err_t ret;
    esp_vfs_spiffs_conf_t conf = {
      .base_path = SPIFFS_BASE_PATH,
      .partition_label = NULL,
      .max_files = 3,
      .format_if_mount_failed = true
    };
    ret = esp_vfs_spiffs_register(&conf); // Will format system (nonblocking) if cannot mount SPIFFS
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

uint32_t jolt_fs_get_all_fns(char **fns, uint32_t fns_len, const char *ext, bool remove_ext){
    /* Returns upto fns_len fns with extension ext and the number of files.
     * If fns is NULL, just return the file count.
     * If ext is NULL, return all files
     * Uses malloc to reserve space for fns
     */
    DIR *dir;
    uint32_t tot = 0;
    struct dirent *ent;
    char *ext_ptr;

    dir = opendir(SPIFFS_BASE_PATH);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open filesystem.");
        fns = NULL;
        return 0;
    }

    // Get file count if fns is NULL
    if(fns == NULL) {
        while((ent = readdir(dir)) != NULL) {
            //ent->d_name is a char array of form "cat.jpg"
            // Check if the file has extension ".elf"
            ext_ptr = ent->d_name + strlen(ent->d_name) - strlen(ext);
            if( !ext || (strlen(ent->d_name)>strlen(ext)  
                        && strcmp(ext_ptr, ext) == 0) ) {
                tot++;
            }
        }
        closedir(dir);
        return tot;
    }

    while((ent = readdir(dir)) != NULL) {
        // Check if the file has extension ".elf"
        ext_ptr = ent->d_name + strlen(ent->d_name) - strlen(ext);
        if( !ext || (strlen(ent->d_name)>strlen(ext)  
                    && strcmp(ext_ptr, ext) == 0) ) {
            uint8_t copy_len = strlen(ent->d_name)+1;
            if( remove_ext ) {
                copy_len -= strlen(ext);
            }
            fns[tot] = malloc(copy_len);
            strlcpy(fns[tot], ent->d_name, copy_len);
            tot++;
            if(tot >= fns_len){
                break;
            }
        }
    }
    closedir(dir);

    return tot;
}

// todo change this to jelf_fns
uint16_t jolt_fs_get_all_elf_fns(char ***fns) {
    uint16_t n;

    n = jolt_fs_get_all_fns(NULL, 0, ".jelf", true);
    ESP_LOGI(TAG, "Found %x apps.", n);
    if( n > 0 ) {
        *fns = jolt_h_malloc_char_array(n);
        jolt_fs_get_all_fns(*fns, n, ".jelf", true);
    }
    return n;
}

uint32_t jolt_fs_free() {
    uint32_t tot, used;
    esp_spiffs_info(NULL, &tot, &used);
    return (tot-used-16384); // todo; is this correct?
}

size_t jolt_fs_size(char *fname) {
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

int jolt_fs_exists(char *fname) {
    if (!esp_spiffs_mounted( NULL )) {
        return -1;
    }

    struct stat sb;
    if (stat(fname, &sb) == 0) {
        return 1;
    }
    return 0;
}


