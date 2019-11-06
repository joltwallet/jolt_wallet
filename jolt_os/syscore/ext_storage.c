#define LOG_LOCAL_LEVEL 4

#include "ext_storage.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdkconfig.h"
#include "sdmmc_cmd.h"
#include "syscore/filesystem.h"

static const char TAG[] = "syscore/ext_storage";

void jolt_ext_storage_init()
{
#if CONFIG_JOLT_EXT_STORAGE_INIT
    ESP_LOG( TAG, "Initializing external storage." );

    sdmmc_host_t host               = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    /* CMD, DAT0, DAT1, DAT2, DAT3 should have external 10k pull-ups
     * Enabling internal pull-ups just "to help" */
    gpio_set_pull_mode( CONFIG_JOLT_EXT_STORAGE_CMD_PIN, GPIO_PULLUP_ONLY );   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode( CONFIG_JOLT_EXT_STORAGE_DAT0_PIN, GPIO_PULLUP_ONLY );  // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode( CONFIG_JOLT_EXT_STORAGE_DAT1_PIN, GPIO_PULLUP_ONLY );  // D1, needed in 4-line mode only
    gpio_set_pull_mode( CONFIG_JOLT_EXT_STORAGE_DAT2_PIN, GPIO_PULLUP_ONLY );  // D2, needed in 4-line mode only
    gpio_set_pull_mode( CONFIG_JOLT_EXT_STORAGE_DAT3_PIN, GPIO_PULLUP_ONLY );  // D3, needed in 4- and 1-line modes

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};

    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount( JOLT_EXT_STORAGE_MOUNTPT, &host, &slot_config, &mount_config, &card );
    if( ret != ESP_OK ) {
        if( ret == ESP_FAIL ) {
            ESP_LOGE( TAG, "Failed to mount filesystem. "
                           "If you want the card to be formatted, set format_if_mount_failed = true." );
        }
        else {
            ESP_LOGE( TAG,
                      "Failed to initialize the card (%s). "
                      "Make sure SD card lines have pull-up resistors in place.",
                      esp_err_to_name( ret ) );
        }
        return;
    }

    sdmmc_card_print_info( stdout, card );

#else
    ESP_LOGD( TAG, "JoltOS not compiled with external storage support." );
#endif
}
