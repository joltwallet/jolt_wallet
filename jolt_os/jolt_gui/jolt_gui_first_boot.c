#include "jolt_gui.h"
#include "jolt_gui_first_boot.h"

#include "bipmnemonic.h"
#include "esp_log.h"
#include "jolttypes.h"

#include "sodium.h"
#include "hal/storage/storage.h"
#include "syscore/bg.h"

#if CONFIG_JOLT_STORE_ATAES132A
#include "aes132_cmd.h"
#endif

#define MNEMONIC_STRENGTH 256

/********************
 * Static Variables *
 ********************/
static const char TAG[] = "first_boot";
static CONFIDENTIAL uint256_t mnemonic_bin;
static CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
static CONFIDENTIAL uint256_t pin_hash;

/********************
 * Static Functions *
 ********************/
static void generate_mnemonic();
static void mismatch_cb(lv_obj_t *btn, lv_event_t event);
static void screen_finish_create(lv_obj_t *verify_scr, lv_event_t event);
static void screen_pin_verify_create(lv_obj_t *pin_scr, lv_event_t event);
static void screen_pin_entry_create(lv_obj_t *btn, lv_event_t event);
static void screen_mnemonic_create(lv_obj_t *btn, lv_event_t event);

static jolt_err_t get_nth_word(char buf[], size_t buf_len,
        const char *str, const uint32_t n);

static jolt_err_t get_nth_word(char buf[], size_t buf_len,
        const char *str, const uint32_t n){
    // Assumes a single space between words; no leading/trailing spaces
    // Copies the nth word of null-terminated str into buf.
    if ( (n+1)==25 ){
        strlcpy(buf, "Continue", buf_len);
        return E_SUCCESS;
    }
    // Copy over number prefix
    sprintf(buf, "%d. ", n+1); // 1-indexing
    buf_len -= 3;
    buf += 3;
    if( (n+1)>9 ){
        buf_len--;
        buf++;
    }
    // Copy over the nth word
    for(uint8_t i = 0; str!=0; str++){
        if( i == n ){
            if(*str == ' ' || *str == '\0' || buf_len <= 1){
                *buf = '\0';
                return E_SUCCESS;
            }
            else{
                *buf = *str;
                buf++;
                buf_len--;
            }
        }
        else if (*str == ' '){
            i++;
        }
    }
    return E_FAILURE;
}

/* Generates 256-bits of entropy into mnemonic_bin.
 * Computes mnemonic string mnemonic from mnemonic_bin. The string is
 * used for display/backup purposes only.*/
static void generate_mnemonic() {
    bm_entropy256(mnemonic_bin);
#if CONFIG_JOLT_STORE_ATAES132A
    {
        /* Mix in entropy from ataes132a */
        CONFIDENTIAL uint256_t aes132_entropy;
        uint8_t res = aes132_rand(aes132_entropy, sizeof(aes132_entropy));
        if( res ) {
            esp_restart();
        }
        for(uint8_t i=0; i<sizeof(aes132_entropy); i++) {
            mnemonic_bin[i] ^= aes132_entropy[i];
        }
        sodium_memzero(aes132_entropy, sizeof(aes132_entropy));
    }
#endif
    bm_bin_to_mnemonic(mnemonic, sizeof(mnemonic), mnemonic_bin,
            MNEMONIC_STRENGTH);
    ESP_LOGI(TAG, "mnemonic %s", mnemonic);
}

/* BG Task for stretching */
static int bg_stretch_task(jolt_bg_job_t *job) {
    int8_t progress = 0;
    lv_obj_t *loading_scr;

    /* Create Loading Bar */
    loading_scr = jolt_gui_scr_loadingbar_create("Saving");
    jolt_gui_scr_loadingbar_autoupdate( loading_scr, &progress );

    /* Perform the lengthy stretching */
    storage_stretch( pin_hash, &progress );

    /* Delete the loading bar screen */
    jolt_gui_obj_del( loading_scr );

    /* Saves setup information to storage */
    storage_set_mnemonic(mnemonic_bin, pin_hash);
    storage_set_pin_count(0); /* Only does something if pin_count is setable */
    uint32_t pin_count = storage_get_pin_count();
    storage_set_pin_last(pin_count);

    sodium_memzero(pin_hash, sizeof(pin_hash));
    sodium_memzero(mnemonic_bin, sizeof(mnemonic_bin));
    sodium_memzero(mnemonic, sizeof(mnemonic));

    esp_restart();

    return 0;
}

static void mismatch_cb(lv_obj_t *btn, lv_event_t event) {
    switch(event) {
        case LV_EVENT_SHORT_CLICKED:
            /* Fall through */
        case LV_EVENT_CANCEL:
            jolt_gui_scr_del();
            screen_pin_entry_create(NULL, LV_EVENT_SHORT_CLICKED);
            break;
        default:
            break;
    }
}

/* Verifies the re-entered pin-hash 
 *     Success: Creates the stretching screen.
 *     Failure: Creates the mismatch screen, sends user back to pin entry.*/
static void screen_finish_create(lv_obj_t *verify_scr, lv_event_t event) {
    switch(event){
        case LV_EVENT_SHORT_CLICKED: {
            int res;
            CONFIDENTIAL static uint256_t pin_hash_verify;

            /* Compute Verify Pin Hash */
            jolt_gui_scr_digit_entry_get_hash(verify_scr, pin_hash_verify);

            // Delete verify pin entry screen
            jolt_gui_obj_del(verify_scr);

            res = memcmp(pin_hash, pin_hash_verify, sizeof(pin_hash_verify));
            sodium_memzero(pin_hash_verify, sizeof(pin_hash_verify));

            /* Verify the pins match */
            if( 0 ==  res ){
                ESP_ERROR_CHECK( jolt_bg_create( bg_stretch_task, NULL, NULL) );
            }
            else{
                sodium_memzero( pin_hash, sizeof(pin_hash) );
                lv_obj_t *scr = jolt_gui_scr_text_create("Pin Setup", "Pin Mismatch! Please try again.");
                jolt_gui_scr_set_event_cb(scr, mismatch_cb);
            }
            break;
        }
        case LV_EVENT_CANCEL:
            jolt_gui_scr_del();
            break;
        default:
            break;
    }
}

/* Saves the entered pinhash. Creates the pin verify screen.
 * Creates StartupScreen 4 
 * */
static void screen_pin_verify_create(lv_obj_t *pin_scr, lv_event_t event) {
    switch(event){
        case LV_EVENT_SHORT_CLICKED: {
            /* Get the has for the first screen */
            jolt_gui_scr_digit_entry_get_hash(pin_scr, pin_hash);
            /* Delete first PIN entry screen */
            jolt_gui_obj_del(pin_scr);

            /* Create Verify PIN screen */
            lv_obj_t *verify_scr = jolt_gui_scr_digit_entry_create( "PIN Verify",
                    CONFIG_JOLT_GUI_PIN_LEN, JOLT_GUI_SCR_DIGIT_ENTRY_NO_DECIMAL); 
            if( NULL == verify_scr ){
                esp_restart();
            }
            jolt_gui_scr_set_event_cb(verify_scr, &screen_finish_create);
            break;
        }
        case LV_EVENT_CANCEL:
            jolt_gui_scr_del();
            break;
        default:
            break;
    }
}

/* Prompts user for pin (first time).
 * Creates StartupScreen3
 */
static void screen_pin_entry_create(lv_obj_t *btn, lv_event_t event) {
    if(event==LV_EVENT_SHORT_CLICKED) {
        lv_obj_t *scr = jolt_gui_scr_digit_entry_create( "PIN",
                CONFIG_JOLT_GUI_PIN_LEN, JOLT_GUI_SCR_DIGIT_ENTRY_NO_DECIMAL);
        if( NULL == scr ){
            esp_restart();
        }
        jolt_gui_scr_set_event_cb(scr, &screen_pin_verify_create);
    }
}

/**
 * Displays 24-word mnemonic.
 * Creates StartupScreen2
 */
static void screen_mnemonic_create(lv_obj_t *btn, lv_event_t event) {
    switch(event){
        case LV_EVENT_SHORT_CLICKED: {
            const char title[] = "Write Down Mnemonic!";
            lv_obj_t *scr = jolt_gui_scr_menu_create(title);
            for(uint8_t i=0; i < 24; i++) {
                char buf[15] = { 0 }; // todo define
                get_nth_word(buf, sizeof(buf), mnemonic, i);
                if( i == 0 ) {
                    jolt_gui_scr_menu_add(scr, NULL, buf, NULL);
                }
                else {
                    jolt_gui_scr_menu_add(scr, NULL, buf, NULL);
                }
            }
            jolt_gui_scr_menu_add(scr, NULL, "continue",
                    screen_pin_entry_create);

            break;
        }
        case LV_EVENT_CANCEL:
            /* Do Nothing - delete action handled by default */
            break;
        default:
            break;
    }
}

/**
 * Called externally to begin the first-boot GUI.
 *
 * Creates StartupScreen1
 */
void jolt_gui_first_boot_create() {
    generate_mnemonic();

    // todo: localization
    lv_obj_t *scr = jolt_gui_scr_text_create( "First Startup",
            "Welcome to Jolt, "
            "please backup the following secret mnemonic.");
    jolt_gui_scr_set_event_cb(scr, &screen_mnemonic_create);
}

/* Prompts user for pin and performs the rest of setup for a given entropy */
void jolt_gui_restore_sequence(const uint256_t mnemonic) {
    memcpy(mnemonic_bin, mnemonic, 32);
    screen_pin_entry_create( NULL, LV_EVENT_SHORT_CLICKED );
}
