/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "jolttypes.h"
#include "bipmnemonic.h"
#include "jolt_helpers.h"
#include "vault.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/bg.h"


/* Overall vault steps 
 * 1. Check to see if vault is valid and params match; if so, kick the dog, then
 *    directly trigger the user's success callback. If not, continue.
 * 2. Set vault to the provided params; keep the semaphore until a pin is entered.
 * 3. Delete the pin screen and provide clean up. Kick the dog again.
 * 4. Call
 *
 */

/* Type Declarations */
/* Objects relevant to the current PIN screen. We can store these globally
 * instead of passing a structure around since there can only be 1 pin screen 
 * at a time */
enum {
    PIN_STATE_FAIL = -1,
    PIN_STATE_EMPTY = 0,
    PIN_STATE_CREATE,
    PIN_STATE_STRETCH,
    PIN_STATE_NUM_STATES, /* Not an actual state */
};
typedef int8_t pin_state_t;

typedef struct pin_store_t {
    /* PIN and derivation Stuff */
    lv_obj_t *scr;
    CONFIDENTIAL uint256_t pin_hash;
    char *mnemonic;

    /* User Stuff */
    vault_cb_t failure_cb;
    vault_cb_t success_cb;
    void *param;

    /* Misc */
    pin_state_t state;
} pin_store_t; /* PIN State */

/* Static Variables */
static const char* TAG = "vault";
static vault_t *vault = NULL;
static pin_store_t ps = { 0 };

/* The following semaphores are not part of the vault object so we can use
 * sodium_malloc() */
static SemaphoreHandle_t vault_sem; // Used for general vault access
static SemaphoreHandle_t vault_watchdog_sem; // Used to kick the dog


/* Static Function */
static void ps_state_cleanup();
static esp_err_t ps_state_exec();
static int ps_state_exec_task( jolt_bg_job_t *job );

static void pin_fail_cb( lv_obj_t *num_entry, lv_event_t event );
static void pin_entry_cb( lv_obj_t *num_entry, lv_event_t event );

static void vault_watchdog_task();

static void ps_state_cleanup() {
    /* Cleanup */
    ESP_LOGV(TAG, "Cleaning up ps_state ");
    if( NULL != ps.scr ) {
        jolt_gui_obj_del( ps.scr );
        ps.scr = NULL;
    }
    sodium_memzero(ps.pin_hash, sizeof(uint256_t));
    ps.failure_cb = NULL;
    ps.success_cb = NULL;
    ps.param      = NULL;
    ps.state      = PIN_STATE_EMPTY;
    vault_sem_give();
}

/* Queues a job to execute ps_state_exec_task */
static esp_err_t ps_state_exec() {
    /* Don't allow bg aborts */
    return jolt_bg_create( ps_state_exec_task, NULL, NULL);
}

/* Increments the PIN Entry State Machine in the BG */
static int ps_state_exec_task( jolt_bg_job_t *job ) {
    /* This always gets executed in the BG Task */
    switch( ps.state ){
        case PIN_STATE_CREATE: {
            /* Create the PIN Screen */ 
            char title[70];

            /* Check PIN Attempts */
            uint32_t pin_attempts = 
                    storage_get_pin_count() - storage_get_pin_last();
            if( pin_attempts >= CONFIG_JOLT_PIN_DEFAULT_MAX_ATTEMPT ) {
                ESP_LOGE(TAG, "MAX PIN Attempts exceeded; factory resetting");
                storage_factory_reset( );
            }

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

            /* Create GUI Objects */
            ESP_LOGD(TAG, "Creating PIN screen");
            JOLT_GUI_CTX{
                ps.scr = BREAK_IF_NULL( jolt_gui_scr_digit_entry_create( 
                        title,
                        CONFIG_JOLT_GUI_PIN_LEN,
                        JOLT_GUI_SCR_DIGIT_ENTRY_NO_DECIMAL ) );
                jolt_gui_scr_set_event_cb(ps.scr, pin_entry_cb);
                ps.state = PIN_STATE_STRETCH;
            }
            break;
        }
        case PIN_STATE_STRETCH:
        {
            /* Converts a pin guess into the mnemonic string.
             * Loops back to PIN_STATE_CREATE on an erroneous PIN attempt. */

            int8_t *progress = NULL;
            lv_obj_t *loading_scr;
            CONFIDENTIAL char mnemonic[265] = { 0 }; // change this to macro
            CONFIDENTIAL uint512_t master_seed;

            /* Get the pin_hash from the pin screen */
            jolt_gui_scr_digit_entry_get_hash(ps.scr, ps.pin_hash);

            /* Delete the Pin Entry Screen */
            jolt_gui_sem_take(); // Prevents flashing screen
            jolt_gui_obj_del( ps.scr );
            ps.scr = NULL;

            /* Create Loading Bar */
            loading_scr = jolt_gui_scr_loadingbar_create(NULL);
            jolt_gui_scr_loadingbar_update(loading_scr,
                    gettext(JOLT_TEXT_PIN),
                    gettext(JOLT_TEXT_CHECKING_PIN), 0);
            jolt_gui_sem_give();
            ESP_LOGD(TAG, "Creating stretch autoupdate lv_task");
            progress = jolt_gui_scr_loadingbar_autoupdate( loading_scr );

            /* Stretch directly here */
            ESP_LOGD(TAG, "Beginning stretch");
            storage_stretch( ps.pin_hash, progress );
            ESP_LOGD(TAG, "Finished stretch");

            /* Delete Loading Screen */
            jolt_gui_sem_take();
            jolt_gui_obj_del( loading_scr );

            /* storage_get_mnemonic inherently increments and stores the 
             * attempt counter*/
            {
                bool unlock_res;
                CONFIDENTIAL uint256_t mnemonic_bin;
                unlock_res = storage_get_mnemonic(mnemonic_bin, ps.pin_hash);
                sodium_memzero( ps.pin_hash, sizeof(uint256_t) );
                if( !unlock_res ) {
                    /* Wrong PIN */
                    ESP_LOGE(TAG, "Incorrect PIN");

                    /* Create a screen telling the user */
                    lv_obj_t *scr = jolt_gui_scr_text_create(
                            gettext(JOLT_TEXT_PIN),
                            gettext(JOLT_TEXT_INCORRECT_PIN));
                    jolt_gui_scr_set_event_cb(scr, pin_fail_cb);

                    jolt_gui_sem_give();
                    ps.state = PIN_STATE_EMPTY;
                    
                    break;
                }
                ESP_LOGI(TAG, "Correct PIN");

                /* Convert Binary Mnemonic to String; Clear Binary */
                bm_bin_to_mnemonic( mnemonic, sizeof(mnemonic),
                        mnemonic_bin, 256);

                /* We no longer need the binary mnemonic */
                sodium_memzero(mnemonic_bin, sizeof(uint256_t));

                ESP_LOGI(TAG, "ClearText Mnemonic: %s", mnemonic);
            }

            /**** We now have mnemonic at this point ****/

            /* Create Loading Bar */
            loading_scr = jolt_gui_scr_loadingbar_create(NULL);
            jolt_gui_scr_loadingbar_update(loading_scr,
                    gettext(JOLT_TEXT_PIN),
                    gettext(JOLT_TEXT_UNLOCKING), 0);
            jolt_gui_sem_give();
            progress = jolt_gui_scr_loadingbar_autoupdate( loading_scr );

            /* Do the lengthy master seed derivation */
            bm_mnemonic_to_master_seed_progress(master_seed, 
                    mnemonic, vault->passphrase, progress);

            /* We no longer need the mnemonic */
            sodium_memzero(mnemonic, sizeof(mnemonic));

            /* Delete Loading Screen */
            jolt_gui_obj_del( loading_scr );


            /**** We now have the master_seed ******/

            /* Derive the coin specific node */
            bm_master_seed_to_node(&(vault->node), 
                    master_seed, vault->bip32_key,
                    2, vault->purpose, vault->coin_type);

            /* We no longer need the master seed */
            sodium_memzero(master_seed, sizeof(master_seed) );

            vault->valid = true;
            sodium_mprotect_readonly(vault);

            /* Cleanup and Call the user callback */
            {
                vault_cb_t cb = ps.success_cb;
                void *param = ps.param;
                
                ps_state_cleanup();

                if( NULL != cb ) {
                    cb( param );
                }
            }

            break;
        }
        case PIN_STATE_FAIL:
            if( NULL != ps.failure_cb ) {
                ps.failure_cb( ps.param );
            }
            else {
                ESP_LOGE(TAG, "No Back Callback");
            }
            ps_state_cleanup();
            break;
        case PIN_STATE_EMPTY:
            ps_state_cleanup();
            break;
        default:
            ps_state_cleanup();
            break;
    }
    return 0;
}

/**
 * @brief Event callback for the pin entrry screen
 */
static void pin_entry_cb( lv_obj_t *num_entry, lv_event_t event) {
    switch(event){
        case LV_EVENT_SHORT_CLICKED:
            /* User pressed enter on the right-most roller */
            if( ps.state != PIN_STATE_FAIL ) {
                ps.state = PIN_STATE_STRETCH;
                ps_state_exec();
            }
            break;
        case LV_EVENT_CANCEL:
            /* User pressed back on the left-most roller */
            if( ps.state != PIN_STATE_FAIL ) {
                ps.state = PIN_STATE_FAIL;
                ps_state_exec();
            }
            break;
        default:
            break;
    }
}

/**
 * @brief Event callback for the "incorrect pin" text screen
 */
static void pin_fail_cb( lv_obj_t *num_entry, lv_event_t event ) {
    switch(event) {
        case LV_EVENT_SHORT_CLICKED:
            /* Fall through */
        case LV_EVENT_CANCEL:
            jolt_gui_scr_del( num_entry );
            if( ps.state != PIN_STATE_FAIL ) {
                ps.state = PIN_STATE_CREATE;
                ps_state_exec();
            }
            break;
        default:
            break;
    }
}

#if ESP_LOG_LEVEL >= ESP_LOG_INFO
static int8_t vault_sem_ctr = 0;
#endif

void vault_sem_take() {
    /* Takes Vault semaphore; restarts device if timesout during take. */
#if ESP_LOG_LEVEL >= ESP_LOG_INFO
    vault_sem_ctr--;
#endif
    ESP_LOGD(TAG, "%s %d", __func__, vault_sem_ctr);

    if( !xSemaphoreTakeRecursive(vault_sem, pdMS_TO_TICKS( 
                    CONFIG_JOLT_VAULT_TIMEOUT_TIMEOUT_MS) ) ) {
        // Timed out trying to take the semaphore; reset the device
        // And let the bootloader wipe the RAM
        ESP_LOGE(TAG, "Failed taking vault semaphore.");
        esp_restart();
    }
}

void vault_sem_give() {
#if ESP_LOG_LEVEL >= ESP_LOG_INFO
    vault_sem_ctr++;
#endif
    ESP_LOGD(TAG, "%s %d", __func__, vault_sem_ctr);
    xSemaphoreGiveRecursive(vault_sem);
}

static void vault_watchdog_task() {
    /* Daemon-like task to wipe vault after a configurable timeout.
     * Should be given very high priority to prevent deadlocks. */
	for(;;) {
        if( xSemaphoreTake( vault_watchdog_sem, 
				pdMS_TO_TICKS(CONFIG_JOLT_VAULT_DEFAULT_TIMEOUT_S * 1000)) ) {
            // Command to reset the private node watchdog;
            // Note, vault_sem is probably taken if in here
            // Do Nothing
        }
        else {
            // WatchDog timeout; wipe the private node
            vault_clear();
        }
	}
}

bool vault_setup() {
    /* Creates the private key object and the watchdog task to monitor it.
     * Also checks to see if there is a stored secret or not.
     *
     * Returns True if a secret (mnemonic) has been previously setup.
     * Returns False if no secret (mnemonic) has been setup.*/
    vault = sodium_malloc(sizeof(vault_t));
    if( NULL==vault ){
        ESP_LOGE(TAG, "Unable to allocate space for the Vault");
        esp_restart();
    }
    /* NOTE: ESP32 doesn't properly implement the mprotect features;
     * awaiting for their implementation for additional security. */
    sodium_mprotect_readwrite(vault);
    sodium_memzero(vault, sizeof(vault_t));
    sodium_mprotect_readonly(vault);

    vault_sem = xSemaphoreCreateRecursiveMutex();
    vault_watchdog_sem = xSemaphoreCreateBinary();

    /* Setup the PIN Entry State Machine */
    ps.scr = NULL;
    memset(ps.pin_hash, 0, sizeof(uint256_t));
    ps.failure_cb = NULL;
    ps.success_cb = NULL;
    ps.param = NULL;
    ps.state = PIN_STATE_EMPTY;

    xTaskCreate(vault_watchdog_task, "VaultWatchDog",
            CONFIG_JOLT_TASK_STACK_SIZE_VAULT_WATCHDOG,
            NULL, CONFIG_JOLT_TASK_PRIORITY_VAULT_WATCHDOG, NULL);

    // Checks if stored secret exists
    return storage_exists_mnemonic();
}

void vault_clear() {
    vault_sem_take();
    if( vault->valid ) {
        ESP_LOGI(TAG, "Clearing Vault.");
        sodium_mprotect_readwrite(vault);
        vault->valid = false;
        sodium_memzero(&(vault->node), sizeof(hd_node_t));
        sodium_mprotect_readonly(vault);
        ESP_LOGI(TAG, "Clearing Vault Complete.");
    }
    vault_sem_give();
}

/********************
 * PUBLIC FUNCTIONS *
 ********************/

/* 
* To only be called before launching an app, or when changing firmware
* settings.
* Since this *may* prompt the user for a PIN, it must be non-blocking
* to agree with the GUI.
*
* If the vault is successfully set, the success callback will be executed.
* On failure to set the vault, the failure callback will be executed.
*
* If passphrase is NULL, the preset passphrase from NVS will be used.
*
*/
esp_err_t vault_set(uint32_t purpose, uint32_t coin_type,
        const char *bip32_key, const char *passphrase,
        vault_cb_t failure_cb, vault_cb_t success_cb, void *param) {

    ESP_LOGD(TAG, "%s", __func__);

    if( PIN_STATE_EMPTY != ps.state ) {
        ESP_LOGE(TAG, "PIN Entry in progress; cannot set vault.");
        return ESP_FAIL;
    }

    if( NULL == passphrase ) {
        /* Populate passphrase here */
        passphrase = "";
    }

    vault_sem_take();

    /* Don't require pin if logging into app with identical vault parameters.
     * Directly call the success callback */
    if( true == vault->valid
            && vault->purpose   == purpose
            && vault->coin_type == coin_type
            && 0 == strcmp(vault->passphrase, passphrase)
            && 0 == strcmp(vault->bip32_key, bip32_key) ) {
        ESP_LOGI(TAG, "%s: identical vault parameters; no PIN required.", __func__);
        vault_kick();
        vault_sem_give();
        success_cb( param );
        return ESP_OK;
    }

    /* Populate Vault with derivation parameters */
    sodium_mprotect_readwrite(vault);
    vault->valid = false;
    vault->purpose = purpose;
    vault->coin_type = coin_type;
    strlcpy( vault->bip32_key, bip32_key, sizeof(vault->bip32_key) );
    strlcpy( vault->passphrase, passphrase, sizeof(vault->passphrase) );
    sodium_mprotect_readonly(vault);

    /* Zero out the pin state */
    memset(&ps, 0, sizeof(ps));
    ps.failure_cb = failure_cb;
    ps.success_cb = success_cb;
    ps.param = param;
    ps.state = PIN_STATE_CREATE;

    /* Note: don't give up vault semaphore here; give it up after
     * pin entry and derivation */

    /* Execute the State Machine */
    return ps_state_exec();
}


/* Kicks dog if vault is valid.
* Repopulates node (therefore prompting user for PIN otherwise
*
* To be called within an app right before a private key is to be used.
*
* Returns true on success,
* false if user cancels (if node needs restored)
*/
void vault_refresh(vault_cb_t failure_cb, vault_cb_t success_cb, void *param) {
    if( vault_kick() ) {
        ESP_LOGI(TAG, "Vault refreshed and valid; calling %p", success_cb);
        if(NULL != success_cb) {
            // todo; execute this on the bg instead
            success_cb(param);
        }
    }
    else if( PIN_STATE_EMPTY != ps.state ) {
        ESP_LOGE(TAG, "PIN Entry in progress; cannot refresh vault.");
        if( NULL != failure_cb ) {
            // todo; execute this on the bg instead
            failure_cb(param);
        }
    }
    else{
        /* Prompt for pin access without overwriting vault values */
        memset(&ps, 0, sizeof(ps));
        ps.state = PIN_STATE_CREATE;
        ps.failure_cb = failure_cb;
        ps.success_cb = success_cb;
        ps.param = param;
        ps_state_exec();
    }
}

bool vault_kick() {
    vault_sem_take();
    if( !vault->valid  ) {
        vault_sem_give();
        return false;
    }
    /* Kick the dog */
    ESP_LOGI(TAG, "Vault is valid; kicking the dog.");
    xSemaphoreGive(vault_watchdog_sem);
    vault_sem_give();
    return true;
}


/********************
 * Standard Getters *
 ********************/

uint32_t vault_get_coin_type(){
    if( NULL != vault ) {
        return vault->coin_type;
    }
    else {
        return 0;
    }
}

uint32_t vault_get_purpose(){
    if( NULL != vault ) {
        return vault->purpose;
    }
    else {
        return 0;
    }
}

char *vault_get_bip32_key(){
    if( NULL != vault ) {
        return vault->bip32_key;
    }
    else {
        return NULL;
    }
}

hd_node_t *vault_get_node(){
    if( NULL != vault ) {
        return &(vault->node);
    }
    else {
        return NULL;
    }
}

bool vault_get_valid(){
    if( NULL != vault ) {
        return vault->valid;
    }
    else {
        return false;
    }
}

