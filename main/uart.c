#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "uart.h"

static const char TAG[] = "uart";


void get_serial_input(char *serial_rx, const int buffersize){
    
    int line_pos = 0;
    
    while(1){
        int c = getchar();
        
        if(c < 0) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        if(c == '\r') {
            continue;
        }
        if(c == '\n') {
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
                ESP_LOGI(TAG, "Command buffer full.");
                serial_rx[line_pos] = '\0';
                break;
            }
        }
    }
}

void get_serial_input_int(char *serial_rx, const int buffersize){
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
                    ESP_LOGI(TAG, "Command buffer full.");
                    serial_rx[line_pos] = '\0';
                    break;
                }
            }
        }
    }
}

void flush_uart(){
    //This is a terrible hack to flush the uarts buffer, a far better option would be rewrite all uart related code
    // to use proper uart code from driver/uart.h
    for(int bad_hack = 0; bad_hack <= 10; bad_hack++){
        getchar();
    };
}

