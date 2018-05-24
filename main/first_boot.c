#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "sodium.h"

#include "easy_input.h"
#include "menu8g2.h"
#include "vault.h"
#include "helpers.h"
#include "secure_entry.h"
#include "globals.h"


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

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue, disp_mutex, NULL, NULL);

    // Generate Mnemonic
    CONFIDENTIAL char mnemonic[MNEMONIC_BUF_LEN];
    nl_mnemonic_generate(mnemonic, MNEMONIC_BUF_LEN, 256);

	for(int8_t current_screen =0;;){
		current_screen = (current_screen<0) ? 0 : current_screen;
		switch(current_screen){
			case(0):
				current_screen += display_welcome(&menu);
				break;
			case(1):{
                const char title[] = "Write Down Mnemonic!";
                if( menu8g2_create_vertical_menu(&menu, title, mnemonic,
                        (void *)&get_nth_word, 25) ){
                    if(menu.index == 24){
                        current_screen++;
                    }
                }
                else{
                    current_screen--;
                }
				break;
			}
			case(2):
                store_mnemonic_reboot(&menu, mnemonic);
                current_screen--;
                break;
			default:
				break;
		}
	}
}
