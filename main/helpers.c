#include "u8g2.h"

uint8_t get_center_x(u8g2_t *u8g2, const char *text){
    // Computes X position to print text in center of screen
    u8g2_uint_t width = u8g2_GetStrWidth(u8g2, text);
    return (u8g2_GetDisplayWidth(u8g2)-width)/2 ;
}

