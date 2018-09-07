/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
https://www.joltwallet.com/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "sodium.h"
#include "easy_input.h"
#include "menu8g2.h"
#include "bipmnemonic.h"

#include "entry.h"
#include "gui.h"
#include "../helpers.h"
#include "../globals.h"
#include "../vault.h"
#include "../hal/storage.h"

#define MNEMONIC_STRENGTH 256

static bool display_welcome(menu8g2_t *menu){
    uint64_t response;
    for(;;){
        response = menu8g2_display_text_title(menu,
                "Welcome to Jolt, please backup the following secret mnemonic.",
                "First Startup");
        if ((1ULL << EASY_INPUT_ENTER) & response){
            return 1;
        }
        else if ((1ULL << EASY_INPUT_BACK) & response){
            return 0;
        }
    }
}

static menu8g2_err_t get_nth_word(char buf[], size_t buf_len,
        const char *str, const uint32_t n){
    // Used as dynamic function to display mnemonic as a menu
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

void first_boot_menu(){
    // Generate Mnemonic
    FULLSCREEN_ENTER;

    CONFIDENTIAL uint256_t mnemonic_bin;
    CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
    bm_entropy256(mnemonic_bin);
#if CONFIG_JOLT_STORE_ATAES132A
    // todo: mix in entropy from ataes132a
#endif
    bm_bin_to_mnemonic(mnemonic, sizeof(mnemonic), mnemonic_bin, MNEMONIC_STRENGTH);

    for(int8_t current_screen =0;;){
        current_screen = (current_screen<0) ? 0 : current_screen;
        switch(current_screen){
            case(0):
                current_screen += display_welcome(menu);
                break;
            case(1):{
                const char title[] = "Write Down Mnemonic!";
                if( menu8g2_create_vertical_menu(menu, title, mnemonic,
                        (void *)&get_nth_word, 25) ){
                    if(menu->index == 24){
                        current_screen++;
                    }
                }
                else{
                    current_screen--;
                }
                break;
            }
            case(2):{
                CONFIDENTIAL uint256_t pin_hash;
                if( entry_verify_pin(menu, pin_hash) ) {
                    storage_set_mnemonic(mnemonic_bin, pin_hash);
                    storage_set_pin_count(0); // Only does something if pin_count is setable
                    uint32_t pin_count = storage_get_pin_count();
                    storage_set_pin_last(pin_count);

                    sodium_memzero(pin_hash, sizeof(pin_hash));
                    sodium_memzero(mnemonic_bin, sizeof(mnemonic_bin));
                    sodium_memzero(mnemonic, sizeof(mnemonic));

                    esp_restart();
                }
                else {
                    current_screen--;
                }
                break;
            }
            default:
                break;
        }
    }
}
