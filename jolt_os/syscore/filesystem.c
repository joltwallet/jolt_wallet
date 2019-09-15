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
    #include "esp_vfs_fat.h"
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

#define MAX_FILES 3

#if CONFIG_JOLT_FS_FAT
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static esp_err_t fat_format();
static esp_err_t setup_fat();
static esp_err_t esp_fatfs_info(size_t* out_total_bytes, size_t* out_free_bytes);
#endif

void jolt_fs_init() {
    esp_err_t ret;

#if CONFIG_JOLT_FS_SPIFFS
    const esp_vfs_spiffs_conf_t conf = {
      .base_path = JOLT_FS_MOUNTPT,
      .partition_label = JOLT_FS_PARTITION,
      .max_files = MAX_FILES,
      .format_if_mount_failed = true
    };
    ret = esp_vfs_spiffs_register(&conf);
#elif CONFIG_JOLT_FS_LITTLEFS 
    const esp_vfs_littlefs_conf_t conf = {
      .base_path = JOLT_FS_MOUNTPT,
      .partition_label = JOLT_FS_PARTITION,
      .max_files = MAX_FILES,
      .format_if_mount_failed = true
    };
    ret = esp_vfs_littlefs_register(&conf);
#elif CONFIG_JOLT_FS_FAT
    const esp_vfs_fat_mount_config_t conf = {
            .max_files = MAX_FILES,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    ret = esp_vfs_fat_spiflash_mount(JOLT_FS_MOUNTPT, JOLT_FS_PARTITION,
            &conf, &s_wl_handle);
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
        *fns = jolt_malloc_char_array(n);
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
    bool res = false;
    struct stat sb;

    char *f = NULL;
    f = jolt_fs_parse(fname, NULL);
    if(NULL == f) return false;

    if (stat(f, &sb) == 0) res = true;
    free(f);
    return res;
}

char * jolt_fs_parse(const char *fn, const char *ext) {
    uint8_t total_len = 1; // NULL-terminator
    bool add_dot = false;
    bool add_mt = false;
    char *path = NULL;

    if( NULL == fn ) return NULL;

    total_len += strlen(fn);

    if( NULL == ext ) ext = &NULL_TERM;
    else if( '.' != ext[0] ){
        add_dot = true;
        total_len++;
    }
    total_len += strlen(ext);

    /* Filename Length Error Check */
    if( JOLT_FS_MAX_FILENAME_LEN < total_len ) {
        ESP_LOGE(TAG, "Parsed path for filename \"%s\" and ext \"%s\" "
                "cumulative len %d exceeds max len %d",
                fn, ext, total_len, JOLT_FS_MAX_FILENAME_LEN);
        return NULL;
    }

    if( 0 != strncmp(fn, JOLT_FS_MOUNTPT "/", sizeof(JOLT_FS_MOUNTPT)) ) {
        add_mt = true;
        total_len += sizeof(JOLT_FS_MOUNTPT); /* mount-point and slash*/
    }

    path = calloc(1, total_len);
    if( NULL == path ){
        ESP_LOGE(TAG, "Error allocating memory for path.");
        return NULL;
    }
    if(add_mt) strlcat(path, JOLT_FS_MOUNTPT "/", total_len);
    strlcat(path, fn, total_len);
    if( add_dot ) strlcat(path, ".", total_len);
    strlcat(path, ext, total_len);

    return path;
}

void jolt_fs_parse_buf(char **fullpath, char **fn){
    char *path;
    *fullpath = calloc(1, JOLT_FS_MAX_ABS_PATH_BUF_LEN);
    path = *fullpath;
    if(NULL == path) return;

    strlcat(path, JOLT_FS_MOUNTPT, JOLT_FS_MAX_ABS_PATH_BUF_LEN);
    strlcat(path, "/", JOLT_FS_MAX_ABS_PATH_BUF_LEN);

    if( NULL == fn ) return;

    *fn = &path[sizeof(JOLT_FS_MOUNTPT)];
}

void jolt_fs_strip_ext(char *fname) {
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.' && *end != '\\' && *end != '/') {
        --end;
    }
    if ((end > fname && *end == '.') &&
        (*(end - 1) != '\\' && *(end - 1) != '/')) {
        *end = '\0';
    }
}

esp_err_t jolt_fs_format(){
#if CONFIG_JOLT_FS_SPIFFS
    return esp_spiffs_format(JOLT_FS_PARTITION);
#elif CONFIG_JOLT_FS_LITTLEFS 
    return esp_littlefs_format(JOLT_FS_PARTITION);
#elif CONFIG_JOLT_FS_FAT
    return fat_format();
#endif
}

esp_err_t jolt_fs_info(size_t *total_bytes, size_t *used_bytes) {
#if CONFIG_JOLT_FS_SPIFFS
    return esp_spiffs_info(JOLT_FS_PARTITION, total_bytes, used_bytes);
#elif CONFIG_JOLT_FS_LITTLEFS 
    return esp_littlefs_info(JOLT_FS_PARTITION, total_bytes, used_bytes);
#elif CONFIG_JOLT_FS_FAT
    return esp_fatfs_info(total_bytes, used_bytes);
#endif
}

esp_err_t jolt_fs_mv(const char *src, const char *dst){

    if( jolt_fs_exists(dst) ) remove(dst);
    if( 0 != rename(src, dst) ) return ESP_FAIL;

    /* Maybe refresh home screen on success */
    jolt_fn_home_refresh( dst );

    return ESP_OK;
}

char *jolt_fs_type(){
#if CONFIG_JOLT_FS_SPIFFS
    return "SPIFFS";
#elif CONFIG_JOLT_FS_LITTLEFS 
    return "LittleFS";
#elif CONFIG_JOLT_FS_FAT
    return "FAT";
#else
    return "UNKNOWN";
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

#if CONFIG_JOLT_FS_FAT
static esp_err_t setup_fat(){
    const esp_vfs_fat_mount_config_t conf = {
            .max_files = MAX_FILES,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(JOLT_FS_MOUNTPT, JOLT_FS_PARTITION, &conf, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        esp_restart();
        return err;
    }
    return ESP_OK;
}

static esp_err_t fat_format(){
    /* Unmount */
    ESP_LOGI(TAG, "Unmounting FAT Filesystem...");
    assert(ESP_OK == esp_vfs_fat_spiflash_unmount(JOLT_FS_MOUNTPT, s_wl_handle));

    /* Wipe Partition */
    ESP_LOGI(TAG, "Wiping Partition...");
    const esp_partition_t *partition;
    partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY,
            JOLT_FS_PARTITION);
    assert( NULL != partition );
    assert( ESP_OK == esp_partition_erase_range(partition, 0, partition->size) );

    /* Remount/Reformat FAT */
    ESP_LOGI(TAG, "Mounting FAT Filesystem...");
    return setup_fat();
}

static esp_err_t esp_fatfs_info(size_t* out_total_bytes, size_t* out_free_bytes){
    FATFS *fs;
    size_t free_clusters;
    int res = f_getfree("0:", &free_clusters, &fs);
    assert(res == FR_OK);
    size_t total_sectors = (fs->n_fatent - 2) * fs->csize;
    size_t free_sectors = free_clusters * fs->csize;

    // assuming the total size is < 4GiB, should be true for SPI Flash
    if (out_total_bytes != NULL) {
        *out_total_bytes = total_sectors * CONFIG_WL_SECTOR_SIZE;
    }
    if (out_free_bytes != NULL) {
        *out_free_bytes = free_sectors * CONFIG_WL_SECTOR_SIZE;
    }
    return ESP_OK;
}
#endif
