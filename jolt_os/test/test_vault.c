#include "bipmnemonic.h"
#include "esp32/clk.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "jolt_gui/jolt_gui.h"
#include "unity.h"
#include "vault.h"

static const char MODULE_NAME[] = "[vault]";

TEST_CASE( "vault_str_to_purpose_type", MODULE_NAME )
{
    uint8_t res;
    uint32_t purpose, coin_type;

    res = vault_str_to_purpose_type( "44'/165'", &purpose, &coin_type );
    TEST_ASSERT_EQUAL_UINT8( 0, res );
    TEST_ASSERT_EQUAL_UINT32( 44 | BM_HARDENED, purpose );
    TEST_ASSERT_EQUAL_UINT32( 165 | BM_HARDENED, coin_type );

    res = vault_str_to_purpose_type( "44/165", &purpose, &coin_type );
    TEST_ASSERT_EQUAL_UINT8( 0, res );
    TEST_ASSERT_EQUAL_UINT32( 44, purpose );
    TEST_ASSERT_EQUAL_UINT32( 165, coin_type );

    res = vault_str_to_purpose_type( "0/0", &purpose, &coin_type );
    TEST_ASSERT_EQUAL_UINT8( 0, res );
    TEST_ASSERT_EQUAL_UINT32( 0, purpose );
    TEST_ASSERT_EQUAL_UINT32( 0, coin_type );
}

TEST_CASE( "vault_set_unit_test", MODULE_NAME ) { vault_set_unit_test( "44'/165'", "ed25519 seed" ); }

static void sem_cb( void *param )
{
    SemaphoreHandle_t sig = param;
    xSemaphoreGive( sig );
}

/***
 * Tests speed of a full vault unlocking procedure.
 *
 * As of the commit right after d3741833 and HW accel: AES
 *  80Mhz - Too slow to reliably run.
 * 160Mhz - 4070mS
 * 240Mhz - 2768mS *** FASTEST ***
 *
 * As of the commit right after d3741833 and HW accel: AES, MPI
 *  80Mhz - Too slow to reliably run.
 * 160Mhz - 3963mS
 * 240Mhz - 2769mS
 *
 * As of the commit right after d3741833 and HW accel: AES, SHA256, MPI
 *  80Mhz - Too slow to reliably run.
 * 160Mhz - 4705mS
 * 240Mhz - 3736mS
 *
 */
TEST_CASE( "Vault speed test", MODULE_NAME )
{
    vault_clear();
    printf( "Current Freq: %d Hz.\n", esp_clk_cpu_freq() );

    SemaphoreHandle_t sig = xSemaphoreCreateBinary();

    uint64_t t_start = esp_timer_get_time();

    jolt_settings_vault_set( NULL, sem_cb, sig );
    for( uint8_t i = 0; i < CONFIG_JOLT_GUI_PIN_LEN; i++ ) {
        vTaskDelay( pdMS_TO_TICKS( 50 ) );
        JOLT_ENTER;
    }
    xSemaphoreTake( sig, portMAX_DELAY );
    uint64_t t_end = esp_timer_get_time();

    printf( "Vault unlocked in %.0f mS\n", ( t_end - t_start ) / 1000.0 );

    vSemaphoreDelete( sig );
}
