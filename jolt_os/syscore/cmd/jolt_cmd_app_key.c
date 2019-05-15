#include "sodium.h"
#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/console_helpers.h"
#include "syscore/launcher.h"
#include "hal/storage/storage.h"

static const char TAG[] = "cmd_app_key";
static uint256_t app_key;

#if 0
static lv_res_t set_app_key_post_factory_reset(lv_obj_t *dummy) {
    if(!storage_set_blob(app_key, sizeof(app_key), "user", "app_key")){
        printf("Error setting app_key.\n");
    }
    printf("Successfully set App Key.\n");
    esp_restart();

    return LV_RES_INV;
}
#endif

static void set_app_key_cb(lv_obj_t *btn, lv_event_t event) {
    if( LV_EVENT_SHORT_CLICKED == event ){
        // todo: sort out factory reset
        //storage_factory_reset( false, set_app_key_post_factory_reset );
    }
    else if(LV_EVENT_CANCEL == event) {
        jolt_gui_scr_del( btn );
    }
}

int jolt_cmd_app_key(int argc, char** argv){
    int return_code = 0;

    /* Input Validation */
    if( !console_check_range_argc(argc, 1, 2) ) {
        return_code = 1;
        goto exit;
    }

    /* Print App Key */
    if( 1 == argc ) {
        uint256_t approved_pub_key;
        hex256_t pub_key_hex;
        size_t required_size;
        if( !storage_get_blob(NULL, &required_size, "user", "app_key") ) {
            printf("Stored App Key not found\n");
            return_code = 4;
            goto exit;
        }
        if( sizeof(approved_pub_key) != required_size ||
                !storage_get_blob(approved_pub_key, &required_size,
                    "user", "app_key")) {
            printf("Stored App Key Blob doesn't have expected len.\n");
            return_code = 5;
            goto exit;
        }
        sodium_bin2hex(pub_key_hex, sizeof(pub_key_hex),
                approved_pub_key, sizeof(approved_pub_key));
        printf("App Key: %s\n", pub_key_hex);
        goto exit;
    }

    /* Set App Key */
    if( strlen(argv[1]) != 64 ) {
        printf("App Key must be 64 characters long in hexadecimal\n");
        return_code = 2;
        goto exit;
    }
    ESP_ERROR_CHECK(sodium_hex2bin(app_key, sizeof(app_key), argv[1], 64,
                NULL, NULL, NULL));

    /* Make sure we are not in an app */
    if( launch_in_app() ) {
        printf("Cannot set app key while an app is running.\n");
        return_code = 3;
        goto exit;
    }

    /* Display Text; Pressing any button returns to previous screen */
    lv_obj_t *scr;
    char body[400];
    snprintf(body, sizeof(body), "WARNING: This will perform a factory reset.\nSet app public key to: \n%s ?", argv[1]);

    /* Prompt user */
    scr = jolt_gui_scr_text_create("Set App Key", body);
    jolt_gui_scr_set_event_cb(scr, set_app_key_cb);

exit:
    return return_code;
}


