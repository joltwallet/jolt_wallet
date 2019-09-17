#include "noise.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "jolt_helpers.h"

#define CONFIG_JOLT_NOISE_AVG_PERIOD_US 100
#define CONFIG_JOLT_NOISE_DELTA         50

/*********************
 * PRIVATE VARIABLES *
 *********************/
static esp_timer_handle_t oneshot_timer;

/*******************************
 * PRIVATE FUNCTION PROTOTYPES *
 *******************************/

static void oneshot_timer_callback();

/********************
 * PUBLIC FUNCTIONS *
 ********************/

void jolt_noise_init()
{
    assert( CONFIG_JOLT_NOISE_AVG_PERIOD_US > CONFIG_JOLT_NOISE_DELTA );

    uint32_t rng;

    const esp_timer_create_args_t oneshot_timer_args = {.callback = &oneshot_timer_callback, .name = "noise"};

    jolt_get_random( (uint8_t *)&rng, sizeof( rng ) );
    srand( rng );

    ESP_ERROR_CHECK( esp_timer_create( &oneshot_timer_args, &oneshot_timer ) );
    ESP_ERROR_CHECK( esp_timer_start_once( oneshot_timer, CONFIG_JOLT_NOISE_AVG_PERIOD_US ) );
}

/*********************
 * PRIVATE FUNCTIONS *
 *********************/
static void oneshot_timer_callback()
{
    // TODO: Verify in hardware this mititgates side-channel attacks.
    uint32_t period = CONFIG_JOLT_NOISE_AVG_PERIOD_US;
    uint32_t rng;

    rng = esp_random();
    period += ( int32_t )( rng % CONFIG_JOLT_NOISE_DELTA ) - CONFIG_JOLT_NOISE_DELTA / 2;

    if( 0 == rng % 17 ) {
        rng = esp_random();
        srand( rng );
    }

    ESP_ERROR_CHECK( esp_timer_start_once( oneshot_timer, period ) );
}
