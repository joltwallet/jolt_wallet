#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "../hal/i2c.h"

static bool test_setup_i2c_flag = false;
void test_setup_i2c() {
    if(!test_setup_i2c_flag) {
        i2c_driver_setup();
    }
    test_setup_i2c_flag=true;
}
