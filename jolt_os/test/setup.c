#include <stdio.h>
#include "../hal/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

void test_setup_i2c()
{
    static bool test_setup_i2c_flag = false;
    if( !test_setup_i2c_flag ) { i2c_driver_setup(); }
    test_setup_i2c_flag = true;
}
