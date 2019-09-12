/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "esp_log.h"

#include "bipmnemonic.h"

#include "syscore/cli.h"
#include "vault.h"
#include "jolt_helpers.h"
#include "sodium.h"

#include "jolt_gui/jolt_gui.h"
#include "syscore/cli_helpers.h"
#include "syscore/bg.h"

/* Communication between jolt/cmd_line inputs and cmd task */
#define MNEMONIC_RESTORE_BACK 0
#define MNEMONIC_RESTORE_ENTER 1
#define MNEMONIC_RESTORE_COMPLETE 2

static const char* TAG = "mnemonic_restore";

enum{
    MNEMONIC_RESTORE_UNDEFINED=0,
    MNEMONIC_RESTORE_CLEANUP,         /**< Indicate to cleanup */
    MNEMONIC_RESTORE_CONFIRM_START,   /**< Waiting on user to confirm they want to proceeed with a restore */
    MNEMONIC_RESTORE_RANDOMIZE,       /**< Generate random mapping*/
    MNEMONIC_RESTORE_24WORDS,         /**< Acquire 24 word mnemonic*/
    MNEMONIC_RESTORE_PROCESS,         /**< Process mnemonic into stored hashes*/
    MNEMONIC_RESTORE_MAX,             /**< Sentinel Value */
};
typedef uint8_t mnemonic_restore_state_t;

typedef struct {
    lv_obj_t *scr;
    int32_t return_code;
    mnemonic_restore_state_t state;
    uint8_t idx;
    CONFIDENTIAL uint8_t mapping[24];
    CONFIDENTIAL char user_words[24][11];
} mnemonic_restore_job_param_t;

/********************
 * STATIC FUNCTIONS *
 ********************/
static void jolt_cmd_mnemonic_restore_num_create( mnemonic_restore_job_param_t *param );
static void jolt_cmd_mnemonic_restore_begin_cb( lv_obj_t *btn, lv_event_t event );
static void jolt_cmd_mnemonic_restore_index_cb( lv_obj_t *btn, lv_event_t event );

static int32_t jolt_mnemonic_restore_process( jolt_bg_job_t *job );


/**
 * @brief Create the screen displaying the word index to enter
 */
static void jolt_cmd_mnemonic_restore_num_create( mnemonic_restore_job_param_t *param ) {
    assert( param->idx < 24 );
    param->scr = jolt_gui_scr_bignum_create( gettext(JOLT_TEXT_RESTORE),
            gettext(JOLT_TEXT_ENTER_MNEMONIC_WORD), param->mapping[param->idx] + 1, -1);
    jolt_gui_scr_set_active_param(param->scr, param);
    jolt_gui_scr_set_event_cb(param->scr, jolt_cmd_mnemonic_restore_index_cb);
}

/**
 * @brief Callback handler for the `begin` screen.
 */
static void jolt_cmd_mnemonic_restore_begin_cb( lv_obj_t *btn, lv_event_t event ) {
    mnemonic_restore_job_param_t *param = NULL;
    param = jolt_gui_obj_get_param( btn );
   
    if( MNEMONIC_RESTORE_CONFIRM_START != param->state ){
        jolt_cli_return(-1);
        return;
    }

    if( LV_EVENT_SHORT_CLICKED == event ) {
        /* Proceed with the mnemonic restore process */
        param->state = MNEMONIC_RESTORE_RANDOMIZE;
        jolt_bg_create( jolt_mnemonic_restore_process, param, NULL);
        jolt_gui_scr_del( btn );
    }
    else if( LV_EVENT_CANCEL == event ) {
        jolt_gui_scr_del( btn );
        jolt_cli_return(-1);
    }
}

/**
 * @brief Callback handler for any "index" screen.
 */
static void jolt_cmd_mnemonic_restore_index_cb( lv_obj_t *btn, lv_event_t event ) {
    mnemonic_restore_job_param_t *param = NULL;
    param = jolt_gui_obj_get_param( btn );
    if( LV_EVENT_SHORT_CLICKED == event ) {
        /* Do Nothing */
    }
    else if( LV_EVENT_CANCEL == event ) {
        param->scr = NULL;
        jolt_gui_scr_del( btn );
        param->return_code = -1;
        param->state = MNEMONIC_RESTORE_CLEANUP;
    }
}

static int32_t jolt_mnemonic_restore_process( jolt_bg_job_t *job ){
    int32_t delay = 100;
    mnemonic_restore_job_param_t *param = NULL;
    param = jolt_bg_get_param(job);
    if( NULL == param ){
        jolt_cli_return(-1);
        return 0;
    }
    
    switch(param->state){
        case MNEMONIC_RESTORE_CONFIRM_START:
            /* Should never execute */
            break;
        case MNEMONIC_RESTORE_CLEANUP:
            /* Clean up */
            if( param->scr ) jolt_gui_obj_del(param->scr);
            jolt_cli_return(param->return_code);
            sodium_memzero(param, sizeof(mnemonic_restore_job_param_t));
            free(param);
            delay = 0;
            break;
        case MNEMONIC_RESTORE_RANDOMIZE:
            /* Generate Random Order for user to input mnemonic */
            for(uint8_t i=0; i< sizeof(param->mapping); i++){
                param->mapping[i] = i;
            }
            shuffle_arr(param->mapping, sizeof(param->mapping));
            param->state = MNEMONIC_RESTORE_24WORDS;
        /* Falls through */
        case MNEMONIC_RESTORE_24WORDS:{
            char *line = NULL;
            int16_t word_idx;
            if( NULL == param->scr ) {
                jolt_cmd_mnemonic_restore_num_create(param);
            }
            line = jolt_cli_get_line(0);
            if( NULL == line ) break;
            if( 0 == strcmp(line, "exit_restore") ) {
                free(line);
                param->state = MNEMONIC_RESTORE_CLEANUP;
                break;
            }
            strlcpy(param->user_words[param->idx], line, sizeof(param->user_words[param->idx]));
            free(line);

            word_idx = bm_search_wordlist(param->user_words[param->idx], strlen(param->user_words[param->idx]));
            if( -1 != word_idx ) {
                /* It's a valid word */
                param->idx++;
                jolt_gui_obj_del(param->scr);
                param->scr = NULL;
            }
            if( param->idx >= 24 ) {
                param->state = MNEMONIC_RESTORE_PROCESS;
                /* Fall Through */
            }
            else {
                break;
            }
        }
        /* Falls through */
        case MNEMONIC_RESTORE_PROCESS:{
            /* Assemble the mnemonic */
            CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
            CONFIDENTIAL uint256_t bin;

            /* Join Mnemonic into single buffer */
            size_t offset=0;
            for(uint8_t i=0; i < 24; i++){
                strlcpy(mnemonic + offset, param->user_words[i], sizeof(mnemonic) - offset);
                offset += strlen(param->user_words[i]);
                mnemonic[offset++] = ' ';
            }
            mnemonic[offset - 1] = '\0'; //null-terminate, remove last space
            ESP_LOGD(TAG, "Entered Mnemonic: %s", mnemonic);

            jolt_err_t err = bm_mnemonic_to_bin(bin, sizeof(bin), mnemonic);
            sodium_memzero(mnemonic, sizeof(mnemonic));
            param->state = MNEMONIC_RESTORE_CLEANUP;
            delay = 1;

            if(E_SUCCESS != err){
                ESP_LOGE(TAG, "Error processing mnemonic.\n");
                break;
            }

            /* Leverages a lot of the same intial-boot code */
            jolt_gui_restore_sequence( bin );

            sodium_memzero(bin, sizeof(bin));

            break;
        }
        default:
            break;
    }
    return delay;
}

int jolt_cmd_mnemonic_restore(int argc, char** argv) {
    lv_obj_t *scr = NULL;
    mnemonic_restore_job_param_t *param = NULL;

    param = malloc(sizeof(mnemonic_restore_job_param_t));
    if( NULL == param ) goto exit;
    memzero(param, sizeof(mnemonic_restore_job_param_t));
    param->state = MNEMONIC_RESTORE_CONFIRM_START;

    /* Create prompt screen */
    JOLT_GUI_CTX{
        scr = BREAK_IF_NULL(jolt_gui_scr_text_create(gettext(JOLT_TEXT_RESTORE),
                gettext(JOLT_TEXT_BEGIN_MNEMONIC_RESTORE)));
        jolt_gui_scr_set_event_cb(scr, jolt_cmd_mnemonic_restore_begin_cb);
        jolt_gui_scr_set_active_param(scr, param);
    }
    if( NULL == scr ) goto exit;

    return JOLT_CLI_NON_BLOCKING;

exit:
    if(param) free(param);
    return -1;

}
