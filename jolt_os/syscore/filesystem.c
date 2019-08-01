/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "sdkconfig.h"

#if CONFIG_JOLT_FS_SPIFFS
    #include "esp_spiffs.h"
#elif CONFIG_JOLT_FS_LITTLEFS 
    #include "esp_littlefs.h"
#elif CONFIG_JOLT_FS_FAT
    assert(0);
#endif

#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "sodium.h"

#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "vault.h"
#include "filesystem.h"
#include "decompress.h"
#include "syscore/cli_helpers.h"

static const char* TAG = "console_syscore_fs";

void jolt_fs_init() {
    esp_err_t ret;

#if CONFIG_JOLT_FS_SPIFFS
    esp_vfs_spiffs_conf_t conf = {
      .base_path = JOLT_FS_MOUNTPT,
      .partition_label = JOLT_FS_PARTITION,
      .max_files = 3,
      .format_if_mount_failed = true
    };
    ret = esp_vfs_spiffs_register(&conf);
#elif CONFIG_JOLT_FS_LITTLEFS 
    esp_vfs_littlefs_conf_t conf = {
      .base_path = JOLT_FS_MOUNTPT,
      .partition_label = JOLT_FS_PARTITION,
      .max_files = 3,
      .format_if_mount_failed = true
    };
    ret = esp_vfs_littlefs_register(&conf);
#elif CONFIG_JOLT_FS_FAT
    assert(0);
#endif

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format filesystem");
        esp_restart();
        return;
    }
    size_t total = 0, used = 0;
    ret = jolt_fs_info(&total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get partition information (%s)", esp_err_to_name(ret));
    }
    else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

uint32_t jolt_fs_get_all_fns(char **fns, uint32_t fns_len, const char *ext, bool remove_ext){
    DIR *dir;
    uint32_t tot = 0;
    struct dirent *ent;
    char *ext_ptr;

    dir = opendir(JOLT_FS_MOUNTPT);
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

uint16_t jolt_fs_get_all_jelf_fns(char ***fns) {
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
    jolt_fs_info(&tot, &used);
    return (tot-used-JOLT_FS_MIN_FREE);
}

size_t jolt_fs_size(const char *fname) {
    struct stat sb;

    if( 0 != stat(fname, &sb) ) {
        return -1;
    }

    return sb.st_size;
}

bool jolt_fs_exists(const char *fname) {
    struct stat sb;
    if (stat(fname, &sb) == 0) {
        return true;
    }
    return false;
}

char * jolt_fs_parse(const char *fn, const char *ext) {
    uint8_t total_len = 0;
    bool add_dot = false;
    char *path = NULL;

    if( NULL == fn ) return NULL;

    total_len += strlen(fn);

    if( NULL == ext ) ext = &NULL_TERM;
    else if( '.' != ext[0] ) add_dot = true;
    total_len += strlen(ext);
    if( add_dot ) total_len += 1;

    /* Filename Length Error Check */
    if( JOLT_FS_MAX_FILENAME_LEN < total_len ) {
        ESP_LOGE(TAG, "Parsed path for filename \"%s\" and ext \"%s\" "
                "cumulative len %d exceeds max len %d",
                fn, ext, total_len, JOLT_FS_MAX_FILENAME_LEN);
        return NULL;
    }

    total_len += sizeof(JOLT_FS_MOUNTPT) + 1; /* mount-point, slash, and NULL-terminator */

    path = calloc(1,total_len);
    if( NULL == path ){
        ESP_LOGE(TAG, "Error allocating memory for path.");
        return NULL;
    }
    strcat(path, JOLT_FS_MOUNTPT);
    if( path[strlen(path)-1] != '/' ) strcat(path, "/");
    strcat(path, fn);
    if( add_dot ) strcat(path, ".");
    strcat(path, ext);

    return path;
}

void jolt_fs_parse_buf(char **fullpath, char **fn){
    char *path;
    *fullpath = calloc(1, JOLT_FS_MAX_ABS_PATH_BUF_LEN);
    path = *fullpath;
    if(NULL == path) return;

    strcat(path, JOLT_FS_MOUNTPT);
    strcat(path, "/");

    if( NULL == fn ) return;

    *fn = &path[sizeof(JOLT_FS_MOUNTPT)];
}

esp_err_t jolt_fs_format(){
#if CONFIG_JOLT_FS_SPIFFS
    return esp_spiffs_format(JOLT_FS_PARTITION);
#elif CONFIG_JOLT_FS_LITTLEFS 
    return esp_littlefs_format(JOLT_FS_PARTITION);
#elif CONFIG_JOLT_FS_FAT
    assert(0); // TODO implement
#endif
}

esp_err_t jolt_fs_info(size_t *total_bytes, size_t *used_bytes) {
#if CONFIG_JOLT_FS_SPIFFS
    return esp_spiffs_info(JOLT_FS_PARTITION, total_bytes, used_bytes);
#elif CONFIG_JOLT_FS_LITTLEFS 
    return esp_littlefs_info(JOLT_FS_PARTITION, total_bytes, used_bytes);
#elif CONFIG_JOLT_FS_FAT
    assert(0);
#endif
}

#if 0
#if CONFIG_JOLT_FS_SPIFFS
    assert(0);
#elif CONFIG_JOLT_FS_LITTLEFS 
    assert(0);
#elif CONFIG_JOLT_FS_FAT
    assert(0);
#endif
#endif
