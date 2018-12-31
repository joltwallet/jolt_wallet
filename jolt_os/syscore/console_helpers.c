#include "stdio.h"
#include "console_helpers.h"

bool console_check_range_argc(uint8_t argc, uint8_t min, uint8_t max){
    if ( argc > max) {
        printf("Too many input arguments; max %d args, got %d\n",
                max, argc);
        return false;
    }
    if ( argc < min) {
        printf("Too few input arguments; min %d args, got %d\n",
                min, argc);
        return false;
    }

    return true;
}

bool console_check_equal_argc(uint8_t argc, uint8_t expected){
    if ( argc != expected) {
        printf("Incorrect number of input arguments; expected %d args, got %d\n",
                expected, argc);
        return false;
    }
    return true;
}


