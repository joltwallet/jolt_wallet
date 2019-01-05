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

#include "console.h"
#include "vault.h"
#include "jolt_helpers.h"
#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "sodium.h"

#include "lvgl.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/console_helpers.h"

/* Communication between jolt/cmd_line inputs and cmd task */
#define MNEMONIC_RESTORE_BACK 0
#define MNEMONIC_RESTORE_ENTER 1
#define MNEMONIC_RESTORE_COMPLETE 2

static const char* TAG = "mnemonic_restore";

static const char title[] = "Restore";
static const char prompt[] = "Enter Mnemonic Word ";

static QueueHandle_t cmd_q;
static CONFIDENTIAL uint8_t idx[24];
static CONFIDENTIAL char user_words[24][11];

static lv_obj_t *jolt_gui_scr_mnemonic_restore_num_create(int n) {
    assert( n <= 24 );
    assert( n >= 1 );

    JOLT_GUI_SCR_CTX(title){
        /* Create text for above the big number */
        lv_obj_t *header_label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
        lv_label_set_text(header_label, "Enter Word");
        lv_obj_align(header_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 6);

        /* Create text for big number */
        char number_str[3] = { 0 }; 
        itoa( n, number_str, 10 );
        lv_obj_t *number_label = BREAK_IF_NULL(lv_label_create(cont_body, NULL));
        lv_label_set_text(number_label, number_str);

        /* Set a Big Font Style for number*/
        static lv_style_t number_style;
        lv_style_t *old_style = lv_label_get_style(number_label);
        lv_style_copy(&number_style, old_style);
        number_style.text.font = &lv_font_dejavu_40_numeric;
        lv_label_set_style(number_label, &number_style);

        /* Align Big Number to Center bottom of screen */
        lv_obj_align(number_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }

    return parent;
}

static lv_res_t jolt_cmd_mnemonic_restore_back( lv_obj_t *btn ) {
    const uint8_t val = MNEMONIC_RESTORE_BACK;
    xQueueSend(cmd_q, (void *) &val, portMAX_DELAY);
    return LV_RES_OK;
}

static lv_res_t jolt_cmd_mnemonic_restore_enter( lv_obj_t *btn ) {
    const uint8_t val = MNEMONIC_RESTORE_ENTER;
    xQueueSend(cmd_q, (void *) &val, portMAX_DELAY);
    return LV_RES_OK;
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
            BREAK_IF_NULL(jolt_gui_scr_set_back_action(scr,  jolt_cmd_mnemonic_restore_back));
            BREAK_IF_NULL(jolt_gui_scr_set_enter_action(scr,  NULL));
        }

        int8_t in_wordlist = 0;
        do {
            if( -1 == in_wordlist ) {
                printf("Invalid word\n");
            }

            printf(prompt);
            jolt_cmd_t cmd_obj;
            jolt_cmd_t *cmd = &cmd_obj;
            xQueueReceive(jolt_cmd_queue, cmd, portMAX_DELAY);
            
            if (strcmp(cmd->data, "exit_restore") == 0){
                printf("Aborting mnemonic restore\n");
                jolt_cmd_del(cmd);
                goto exit;
            }

            strlcpy(user_words[j], cmd->data, sizeof(user_words[j]));
            jolt_cmd_del(cmd);
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

    cmd_q = xQueueCreate( 3, sizeof( uint8_t  )  );

    /* Create prompt screen */
    printf("To begin mnemonic restore, approve prompt on device.\n");
    lv_obj_t *scr = NULL, *btn = NULL;
    JOLT_GUI_CTX{
        scr = BREAK_IF_NULL(jolt_gui_scr_text_create(title, "Begin mnemonic restore?"));
        btn = BREAK_IF_NULL(jolt_gui_scr_set_back_action(scr,  jolt_cmd_mnemonic_restore_back));
        btn = BREAK_IF_NULL(jolt_gui_scr_set_enter_action(scr, jolt_cmd_mnemonic_restore_enter));
    }
    if( NULL == scr && NULL == btn ){
        JOLT_GUI_CTX{
            lv_obj_del(scr);
        }
        return_code = -1;
        goto exit;
    }

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

    /* Generate Random Order for user to input mnemonic */
    for(uint8_t i=0; i< sizeof(idx); i++){
        idx[i] = i;
    }
    shuffle_arr(idx, sizeof(idx));

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

    // Join Mnemonic into single buffer
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
    JOLT_GUI_CTX{
        jolt_gui_restore_sequence( bin );
    }

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


