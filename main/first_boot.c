#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "sodium.h"

#include "easy_input.h"
#include "menu8g2.h"
#include "vault.h"
#include "secure_entry.h"
#include "globals.h"


static bool array_match(unsigned char *arr1, unsigned char *arr2, uint8_t len){
    for(int i=0; i<len; i++){
        if(arr1[i] != arr2[i]){
            return false;
        }
    }
    return true;
}

static bool display_welcome(menu8g2_t *menu){
    uint64_t response;
    for(;;){
        response = menu8g2_display_text(menu, "Welcome!");
        if (1ULL << EASY_INPUT_ENTER == response){
            return 1;
        }
        else if (1ULL << EASY_INPUT_BACK == response){
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

void first_boot_menu(vault_t *vault){
    /* This and vault_task are the only functions that can directly access 
     * the vault */

	int8_t current_screen = 0;
    CONFIDENTIAL uint256_t pin_hash;
    CONFIDENTIAL uint256_t pin_hash_verify;
	uint256_t nonce = {0};
	CONFIDENTIAL unsigned char enc_mnemonic[crypto_secretbox_MACBYTES + MNEMONIC_BUF_LEN];

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue, disp_mutex);

    // Generate Mnemonic
    sodium_mprotect_readwrite(vault);
    nl_mnemonic_generate(vault->mnemonic, MNEMONIC_BUF_LEN, 256);
    vault->valid = false; // To force proper vault update later
    vault->index = 0;
    sodium_mprotect_readonly(vault);

	for(bool exit_welcome=false; !exit_welcome;){
		current_screen = (current_screen<0) ? 0 : current_screen;
		switch(current_screen){
			case(0):
				current_screen += display_welcome(&menu);
				break;
			case(1):{
                const char title[] = "Write Down Mnemonic!";
                if( menu8g2_create_vertical_menu(&menu, title, vault->mnemonic,
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
				while(true){
                    if( !pin_entry(&menu, pin_hash, "Set PIN") ){
                        current_screen--;
                        break;
                    }
					if( !pin_entry(&menu, pin_hash_verify, "Verify PIN")){
                        break;
                    }

					// Verify the pins match
					if(array_match(pin_hash, pin_hash_verify, 32)){
						current_screen++;
						sodium_memzero(pin_hash_verify, 32);
						break;
					}
					else{
						menu8g2_display_text(&menu, "Pin mismatch");
					}
				}
				break;
			case(3):
				exit_welcome = true;
				break;
			default:
				break;
		}
	}

	// Nonce is sort of irrelevant for this encryption
	crypto_secretbox_easy(enc_mnemonic, (unsigned char *) vault->mnemonic, 
            MNEMONIC_BUF_LEN, nonce, pin_hash);
	sodium_memzero(pin_hash, 32);

	// Save everything to NVS
	nvs_handle h;
    init_nvm_namespace(&h, "secret");
	nvs_set_blob(h, "mnemonic", enc_mnemonic, sizeof(enc_mnemonic));
    nvs_set_u8(h, "pin_attempts", 0);
    nvs_set_u32(h, "index", 0);
	nvs_commit(h);
    nvs_close(h);
	sodium_memzero(enc_mnemonic, sizeof(enc_mnemonic));

    sodium_mprotect_noaccess(vault);
}
