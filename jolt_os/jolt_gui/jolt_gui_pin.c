#include "jolt_gui_pin.h"
#include "jolt_gui.h"
#include "hal/storage/storage.h"
#include "sodium.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "syscore/launcher.h"
#include "vault.h"

static const char TAG[] = "JoltGuiPIN";
static lv_action_t user_back_cb;
static lv_action_t user_enter_cb;

/* Gets triggered when user presses back on the "incorrect pin" screen */
static lv_res_t pin_fail_cb( lv_obj_t *btn ) {
    /* Recreate the pin entry screen */
    jolt_gui_scr_pin_create(user_back_cb, user_enter_cb);
    /* Delete the "incorrect pin" screen */
    lv_obj_t *scr = lv_obj_get_parent(btn);
    ESP_LOGI(TAG, "Deleting %p", scr);
    lv_obj_del(scr);
    return LV_RES_INV;
}

static lv_res_t stretch_cb(lv_obj_t *btn) {
    /* storage_get_mnemonic inherently increments and stores the 
     * attempt counter*/
    bool unlock_res = storage_get_mnemonic(
            jolt_gui_store.derivation.mnemonic_bin, 
            jolt_gui_store.derivation.pin);
    sodium_memzero( jolt_gui_store.derivation.pin,
            sizeof(jolt_gui_store.derivation.pin) );
    if( unlock_res ) {
        /* Correct PIN */
        ESP_LOGI(TAG, "Correct PIN");

        /* Execute the user callback */
        if( NULL != user_enter_cb ) {
            user_enter_cb( NULL );
        }
        else {
            ESP_LOGE(TAG, "No Enter Callback");
        }
    }
    else{
        /* Wrong PIN */
        ESP_LOGE(TAG, "Incorrect PIN");

        /* Create a screen telling the user */
        lv_obj_t *scr = jolt_gui_scr_text_create("PIN", "Wrong PIN");
        ESP_LOGI(TAG, "Created Wrong PIN Screen at %p", scr);
        jolt_gui_scr_set_back_action(scr, pin_fail_cb);
        jolt_gui_scr_set_enter_action(scr, pin_fail_cb);
    }
    return LV_RES_OK;
}

/* gets triggered when the user presses enter on the last pin roller on a pin
 * screen */
static lv_res_t pin_enter_cb(lv_obj_t *num) {
    // todo: display derivation path
    // todo: actually make this a loading screen
    
    /* Get the pin_hash from the pin screen */
    jolt_gui_num_get_hash(num, jolt_gui_store.derivation.pin);

    /* Delete the Pin Entry Screen */
    ESP_LOGI(TAG, "Deleting PIN Screen");
    lv_obj_del(lv_obj_get_parent(num));

    ESP_LOGI(TAG, "Calling Storage Stretch");
    // todo: use app name for title
    jolt_gui_stretch("", "Checking PIN", jolt_gui_store.derivation.pin, stretch_cb);

    return LV_RES_INV;
}

static lv_res_t pin_back_cb( lv_obj_t *num ) {
    lv_obj_del(lv_obj_get_parent(num));
    if( NULL != user_back_cb ) {
        user_back_cb( NULL );
    }
    else {
        ESP_LOGE(TAG, "No Back Callback");
    }
    return LV_RES_INV;
}

lv_obj_t *jolt_gui_scr_pin_create(lv_action_t failure_cb, lv_action_t success_cb) {
    /* Primary Job: Prompt user for a pin, and on success to put the 
     * 256-bit mnemonic into jolt_gui_store.derivation.mnemonic_bin.
     *
     * It is host function's responsibility to ensure cleanup of mnemonic.
     *
     * Executes failure_cb if the user backs out of the pin entry screen.
     * Executes the success_cb if pin was correctly entered */
    char title[70];
    uint32_t pin_attempts = storage_get_pin_count() - storage_get_pin_last();
    if( pin_attempts >= CONFIG_JOLT_PIN_DEFAULT_MAX_ATTEMPT ) {
        storage_factory_reset( true, NULL );
    }
    /* Store the callbacks */
    user_back_cb = failure_cb;
    user_enter_cb = success_cb;

    /* Assemble Title */
    #if CONFIG_JOLT_PIN_TITLE_PIN
    {
        sprintf(title, "PIN (%d/%d)", pin_attempts+1,
                CONFIG_JOLT_PIN_DEFAULT_MAX_ATTEMPT);
    }
    #elif CONFIG_JOLT_PIN_TITLE_NAME
    {
        char *app_name = launch_get_name();
        sprintf(title, "%s (%d/%d)", app_name,
                pin_attempts+1, CONFIG_JOLT_PIN_DEFAULT_MAX_ATTEMPT);
    }
    #elif CONFIG_JOLT_PIN_TITLE_PATH
    {
        uint32_t purpose = vault_get_purpose();
        uint32_t type = vault_get_coin_type();
        char *c = title;
        c += sprintf(c, "%d", purpose & ~BM_HARDENED);
        if(purpose & BM_HARDENED ){
            *c = '\'';
            c++;
        }
        c += sprintf(c, "/%d", type & ~BM_HARDENED);
        if(type & BM_HARDENED ){
            *c = '\'';
            c++;
        }
        sprintf(c, " (%d/%d)", pin_attempts+1,
                CONFIG_JOLT_PIN_DEFAULT_MAX_ATTEMPT);
    }
    #endif

    lv_obj_t *parent = NULL;
    JOLT_GUI_CTX{
        lv_obj_t *parent = jolt_gui_scr_num_create( title,
                CONFIG_JOLT_GUI_PIN_LEN, JOLT_GUI_NO_DECIMAL, pin_enter_cb);
        lv_obj_t *numeric = jolt_gui_scr_num_get(parent);
        jolt_gui_num_set_back_action(numeric, pin_back_cb);
    }

    return parent;
}
