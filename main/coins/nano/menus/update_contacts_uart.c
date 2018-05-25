#include "esp_log.h"
#include "menu8g2.h"
#include "../../../vault.h"
#include "submenus.h"
#include "../../../loading.h"
#include "../../../helpers.h"
#include "../contacts.h"
#include "nano_lib.h"

static void get_serial_input(char *serial_rx, int buffersize){
    
    int line_pos = 0;
    
    while(1){
        int c = getchar();
        
        if(c < 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        if(c == '\n' || c == '\r') {
            
            // terminate the string
            serial_rx[line_pos] = '\0';
            printf("\n");
            break;
        }
        else {
            putchar(c);
            serial_rx[line_pos] = c;
            line_pos++;
            
            // buffer full!
            if(line_pos == buffersize) {
                
                printf("\nCommand buffer full!\n");
                serial_rx[line_pos] = '\0';
                
                break;
            }
        }
        
        
    }
}

static void get_serial_input_int(char *serial_rx, const int buffersize){
    // fills up serial_rx with an ascii string where all characters must be ints
    
    int line_pos = 0;
    
    while(1){
        int c = getchar();
        
        if(c < 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        if(c == '\r') continue;
        if(c == '\n') {
            // terminate the string
            serial_rx[line_pos] = '\0';
            printf("\n");
            break;
        }
        else {
            if( c >= '0' && c <= '9' ){
                putchar(c);
                serial_rx[line_pos] = c;
                line_pos++;
                
                // buffer full!
                if(line_pos == buffersize) {
                    printf("\nCommand buffer full!\n");
                    serial_rx[line_pos] = '\0';
                    break;
                }
            }
        }
    }
}

static void flush_uart(){
    //This is a terrible hack to flush the uarts buffer, a far better option would be rewrite all uart related code
    // to use proper uart code from driver/uart.h
    for(int bad_hack = 0; bad_hack <= 10; bad_hack++){
        getchar();
    };
}

void menu_nano_update_contact_uart(menu8g2_t *prev){
    const char title[] = "Nano Contacts";
    bool res;
    char buf[100] ;

    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    menu.post_draw = NULL;
   
    esp_log_level_set("*", ESP_LOG_ERROR);

    loading_enable();
    loading_text("Contact Slot");
    flush_uart();
    printf("\nEnter Nano Contact Addres Slot (int >=0): ");
    get_serial_input_int(buf, sizeof(buf));
    uint8_t contact_index = atoi(buf);
    if( contact_index >= CONFIG_JOLT_NANO_CONTACTS_MAX ){
        snprintf(buf, sizeof(buf), "Contact index must be smaller than %d.",
                CONFIG_JOLT_NANO_CONTACTS_MAX);
        menu8g2_display_text_title(&menu, buf, title);
        return;
    }

    char name[CONFIG_JOLT_NANO_CONTACTS_NAME_LEN];
    flush_uart();
    printf("\nContact Name: ");
    get_serial_input(name, sizeof(name));

    uint256_t contact_public_key;
    flush_uart();
    printf("\nNano Address: ");
    get_serial_input(buf, sizeof(buf));
    if( E_SUCCESS != nl_address_to_public(contact_public_key, buf) ){
        menu8g2_display_text_title(&menu, "Invalid Address", title);
        return;
    }
   
    loading_disable();
    esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);

    vault_rpc_t rpc = {
        .type = NANO_CONTACT_UPDATE,
        .nano_contact_update.name = name,
        .nano_contact_update.index = contact_index,
        .nano_contact_update.public = contact_public_key
    };

    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }
}
