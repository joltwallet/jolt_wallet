/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include "bipmnemonic.h"

#include "syscore/cli.h"
#include "vault.h"
#include "jolt_helpers.h"
#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "sodium.h"

#include "lvgl/lvgl.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/console_helpers.h"

/* Communication between jolt/cmd_line inputs and cmd task */
#define MNEMONIC_RESTORE_BACK 0
#define MNEMONIC_RESTORE_ENTER 1
#define MNEMONIC_RESTORE_COMPLETE 2

static const char* TAG = "mnemonic_restore";

#if 0
enum{
    MNEMONIC_RESTORE_UNDEFINED=0,
    MNEMONIC_RESTORE_CONFIRM_START, /**< */\
    MNEMONIC_RESTORE_RANDOMIZE,     /**< */\
    MNEMONIC_RESTORE_24WORDS,       /**< */\
    MNEMONIC_RESTORE_PROCESS,       /**< */\
    MNEMONIC_RESTORE_MAX,
};
typedef uint8_t mnemonic_restore_state_t;

typedef struct {
    mnemonic_restore_state_t state;
    CONFIDENTIAL uint8_t idx[24];
    CONFIDENTIAL char user_words[24][11];
} mnemonic_restore_job_param_t;

static lv_obj_t *jolt_gui_scr_mnemonic_restore_num_create(int n) {
    assert( n <= 24 );
    assert( n >= 1 );
    return jolt_gui_scr_bignum_create(gettext(JOLT_TEXT_RESTORE), gettext(JOLT_TEXT_ENTER_MNEMONIC_WORD), n, -1);
}

static void jolt_cmd_mnemonic_restore_cb( lv_obj_t *btn, lv_event_t event ) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        const uint8_t val = MNEMONIC_RESTORE_ENTER;
        xQueueSend(cmd_q, (void *) &val, portMAX_DELAY);
    }
    else if( LV_EVENT_CANCEL == event ) {
        const uint8_t val = MNEMONIC_RESTORE_BACK;
        xQueueSend(cmd_q, (void *) &val, portMAX_DELAY);
    }
}


static int32_t jolt_mnemonic_restore_task( jolt_bg_job_t *job){
    mnemonic_restore_job_param_t *param = NULL;
    param = jolt_bg_get_param(job);
    if( NULL == param ){
        param = malloc(sizeof mnemonic_restore_job_param_t);
        if( NULL == param ) {
            // todo error handling
        }
        param->state = MNEMONIC_RESTORE_RANDOMIZE;
    }
    
    switch(param->state){
        case MNEMONIC_RESTORE_CONFIRM_START:
            break;
        case MNEMONIC_RESTORE_RANDOMIZE:
            /* Generate Random Order for user to input mnemonic */
            for(uint8_t i=0; i< sizeof(param->idx); i++){
                param->idx[i] = i;
            }
            shuffle_arr(param->idx, sizeof(param->idx));
            param->MNEMONIC_RESTORE_24WORDS
            /* Fall Through */
        case MNEMONIC_RESTORE_24WORDS:
            break;
        case MNEMONIC_RESTORE_PROCESS:
            break;
        default:
            break;
    }
}




/* Goal: populate the 24 words in ordered */
static void linenoise_task( void *h ) {
    uint8_t val_to_send = MNEMONIC_RESTORE_BACK;
    for(uint8_t i=0; i < sizeof(idx); i++){
        uint8_t j = idx[i];
        lv_obj_t *scr = NULL;

        /* Create Jolt Screen */ 
        JOLT_GUI_CTX{
            jolt_gui_scr_del();
            scr = BREAK_IF_NULL(jolt_gui_scr_mnemonic_restore_num_create(j+1)); /*Human-friendly 1-idxing */
            jolt_gui_scr_set_event_cb(scr,  jolt_cmd_mnemonic_restore_cb);
        }

        int8_t in_wordlist = 0;
        do {
            if( -1 == in_wordlist ) {
                printf("Invalid word\n");
            }

            printf(gettext(JOLT_TEXT_ENTER_MNEMONIC_WORD));
            char *line = linenoise(" > ");

            if (line == NULL) { /* Ignore empty lines */
                continue;
            }

            if (strcmp(line, "exit_restore") == 0){
                printf("Aborting mnemonic restore");
                printf("\n");
                free(line);
                goto exit;
            }

            strlcpy(user_words[j], line, sizeof(user_words[j]));
            free(line);

            in_wordlist = bm_search_wordlist(user_words[j], strlen(user_words[j]));
        }while( -1 == in_wordlist );
    }
    val_to_send = MNEMONIC_RESTORE_COMPLETE;

exit:
    sodium_memzero(idx, sizeof(idx));
    xQueueSend( cmd_q, (void *) &val_to_send, portMAX_DELAY );
    *(TaskHandle_t *)h = NULL;
    vTaskDelete(NULL);
}

int jolt_cmd_mnemonic_restore(int argc, char** argv) {
    int return_code = 0;
    TaskHandle_t linenoise_h = 0;
    CONFIDENTIAL uint256_t bin;
    memset( idx, 0, sizeof( idx ) );
    memset(user_words, 0, sizeof(user_words));

    /* Create prompt screen */
    printf("To begin mnemonic restore, approve prompt on device.\n");
    lv_obj_t *scr = NULL;
    JOLT_GUI_CTX{
        scr = BREAK_IF_NULL(jolt_gui_scr_text_create(gettext(JOLT_TEXT_RESTORE),
                gettext(JOLT_TEXT_BEGIN_MNEMONIC_RESTORE)));
        jolt_gui_scr_set_event_cb(scr, jolt_cmd_mnemonic_restore_cb);
    }
    if( NULL == scr ){
        jolt_gui_obj_del(scr);
        return_code = -1;
        goto exit;
    }

    jolt_bg_create(jolt_mnemonic_restore_task, NULL, NULL);




    /* Wait until user made a choice */
    uint8_t response;
    xQueueReceive(cmd_q, &response, portMAX_DELAY);
    if( MNEMONIC_RESTORE_BACK == response ){
        /* Delete prompt screen and exit */
        JOLT_GUI_CTX{
            jolt_gui_scr_del();
        }
        return_code = -1;
        goto exit;
    }


    /* Create cmd-line monitoring task */
    xTaskCreate(linenoise_task, "m_restore_cmd", 4096,
            (void*)&linenoise_h, 10, &linenoise_h);

    /* Wait for entry completion or cancellation */
    xQueueReceive(cmd_q, &response, portMAX_DELAY);
    if( MNEMONIC_RESTORE_BACK == response ) {
        jolt_gui_scr_del();
        return_code = -1;
        goto exit;
    }
    else if ( MNEMONIC_RESTORE_COMPLETE == response ) {
    }

    /* Join Mnemonic into single buffer */
    CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
    size_t offset=0;
    for(uint8_t i=0; i < sizeof(idx); i++){
        strlcpy(mnemonic + offset, user_words[i], sizeof(mnemonic) - offset);
        offset += strlen(user_words[i]);
        mnemonic[offset++] = ' ';
    }
    mnemonic[offset - 1] = '\0'; //null-terminate, remove last space

    jolt_err_t err = bm_mnemonic_to_bin(bin, sizeof(bin), mnemonic);
    if(E_SUCCESS != err){
        ESP_LOGE(TAG, "Error processing mnemonic.\n");
        goto exit;
    }
    sodium_memzero(mnemonic, sizeof(mnemonic));

    /* Leverages a lot of the same intial-boot code */
    jolt_gui_restore_sequence( bin );

exit:
    printf("Exiting Mnemonic Restore.\n");
    if( 0 != linenoise_h ) {
        vTaskDelete(linenoise_h);
    }
    vQueueDelete(cmd_q);
    sodium_memzero(idx, sizeof(idx));
    sodium_memzero(mnemonic, sizeof(mnemonic));
    sodium_memzero(bin, sizeof(bin));
    return return_code;
}
#else

int jolt_cmd_mnemonic_restore(int argc, char** argv) {
    /* Stub */
    return 0;
}

#endif
