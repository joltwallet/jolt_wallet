#include "cli_helpers.h"
#include "esp_log.h"
#include "stdio.h"

static const char TAG[] = "cli_helpers";

bool console_check_range_argc( uint8_t argc, uint8_t min, uint8_t max )
{
    if( argc > max ) {
        ESP_LOGE( TAG, "Too many input arguments; max %d args, got %d\n", max, argc );
        return false;
    }
    if( argc < min ) {
        ESP_LOGE( TAG, "Too few input arguments; min %d args, got %d\n", min, argc );
        return false;
    }

    return true;
}

bool console_check_equal_argc( uint8_t argc, uint8_t expected )
{
    if( argc != expected ) {
        ESP_LOGE( TAG, "Incorrect number of input arguments; expected %d args, got %d\n", expected, argc );
        return false;
    }
    return true;
}
