#ifndef __JOLT_LIB_H__
#define __JOLT_LIB_H__

#define EXPORT_SYMBOL(NAME) {#NAME, (void*) NAME}
#define EXPORT_SYMBOL_PTR(NAME) {#NAME, (void*) &NAME}

#include "elfloader.h"

#include "bipmnemonic.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "linenoise/linenoise.h"
#include "math.h"
#include "mbedtls/bignum.h"
#include "nano_rest.h"
#include "qrcode.h"
#include "sodium.h"
#include "sodium/crypto_verify_32.h"
#include "sodium/private/curve25519_ref10.h"

#include "console.h"
#include "jolt_globals.h"
#include "hal/storage/storage.h"
#include "jolt_helpers.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/filesystem.h"
#include "vault.h"

extern void *__floatsidf;
extern void *__gtdf2;
extern void *__ltdf2;
extern void *__muldf3;
extern void *__stack_chk_fail;
extern void *__stack_chk_guard;

const ELFLoaderSymbol_t exports[] = {
    EXPORT_SYMBOL_PTR( __floatsidf ),
    EXPORT_SYMBOL_PTR( __gtdf2 ),
    EXPORT_SYMBOL_PTR( __ltdf2 ),
    EXPORT_SYMBOL_PTR( __muldf3 ),
    EXPORT_SYMBOL_PTR( __stack_chk_fail ),
    EXPORT_SYMBOL_PTR( __stack_chk_guard ),
    EXPORT_SYMBOL( _esp_error_check_failed ),
    EXPORT_SYMBOL( atoi ),
    EXPORT_SYMBOL( atol ),
    EXPORT_SYMBOL( bm_entropy256 ),
    EXPORT_SYMBOL( cJSON_Delete ),
    EXPORT_SYMBOL( cJSON_GetObjectItemCaseSensitive ),
    EXPORT_SYMBOL( cJSON_IsString ),
    EXPORT_SYMBOL( cJSON_Parse ),
    EXPORT_SYMBOL( cJSON_Print ),
    EXPORT_SYMBOL( console_check_equal_argc ),
    EXPORT_SYMBOL( console_check_range_argc ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_double_scalarmult_vartime ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_frombytes_negate_vartime ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_p3_tobytes ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_p3_tobytes ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_scalarmult_base ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_tobytes ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_sc_muladd ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_sc_reduce ),
    EXPORT_SYMBOL( crypto_generichash ),
    EXPORT_SYMBOL( crypto_generichash_blake2b ),
    EXPORT_SYMBOL( crypto_generichash_blake2b_final ),
    EXPORT_SYMBOL( crypto_generichash_blake2b_init ),
    EXPORT_SYMBOL( crypto_generichash_blake2b_update ),
    EXPORT_SYMBOL( crypto_generichash_final ),
    EXPORT_SYMBOL( crypto_generichash_init ),
    EXPORT_SYMBOL( crypto_generichash_update ),
    EXPORT_SYMBOL( crypto_hash_sha512_final ),
    EXPORT_SYMBOL( crypto_hash_sha512_init ),
    EXPORT_SYMBOL( crypto_hash_sha512_update ),
    EXPORT_SYMBOL( crypto_verify_32 ),
    EXPORT_SYMBOL( esp_console_deinit ),
    EXPORT_SYMBOL( esp_console_init ),
    EXPORT_SYMBOL( esp_log_timestamp ),
    EXPORT_SYMBOL( esp_log_write ),
    EXPORT_SYMBOL( esp_restart ),
    EXPORT_SYMBOL( free ),
    EXPORT_SYMBOL( get_display_brightness ),
    EXPORT_SYMBOL( hd_node_copy ),
    EXPORT_SYMBOL( hd_node_iterate ),
    EXPORT_SYMBOL( heap_caps_calloc ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_add ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_create ),
    EXPORT_SYMBOL( linenoise ),
    EXPORT_SYMBOL( malloc ),
    EXPORT_SYMBOL( mbedtls_mpi_add_abs ),
    EXPORT_SYMBOL( mbedtls_mpi_add_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_cmp_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_copy ),
    EXPORT_SYMBOL( mbedtls_mpi_free ),
    EXPORT_SYMBOL( mbedtls_mpi_init ),
    EXPORT_SYMBOL( mbedtls_mpi_lset ),
    EXPORT_SYMBOL( mbedtls_mpi_read_string ),
    EXPORT_SYMBOL( mbedtls_mpi_sub_abs ),
    EXPORT_SYMBOL( mbedtls_mpi_sub_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_write_binary ),
    EXPORT_SYMBOL( mbedtls_mpi_write_string ),
    EXPORT_SYMBOL( memchr ),
    EXPORT_SYMBOL( memcmp ),
    EXPORT_SYMBOL( memcpy ),
    EXPORT_SYMBOL( memmove ),
    EXPORT_SYMBOL( memset ),
    EXPORT_SYMBOL( network_get_data ),
    EXPORT_SYMBOL( printf ),
    EXPORT_SYMBOL( puts ),
    EXPORT_SYMBOL( qrcode_getBufferSize ),
    EXPORT_SYMBOL( qrcode_initText ),
    EXPORT_SYMBOL( randombytes_random ),
    EXPORT_SYMBOL( snprintf ),
    EXPORT_SYMBOL( sodium_bin2hex ),
    EXPORT_SYMBOL( sodium_hex2bin ),
    EXPORT_SYMBOL( sodium_malloc ),
    EXPORT_SYMBOL( sodium_memcmp ),
    EXPORT_SYMBOL( sodium_memzero ),
    EXPORT_SYMBOL( sscanf ),
    EXPORT_SYMBOL( storage_erase_key ),
    EXPORT_SYMBOL( storage_get_blob ),
    EXPORT_SYMBOL( storage_get_str ),
    EXPORT_SYMBOL( storage_get_u16 ),
    EXPORT_SYMBOL( storage_get_u32 ),
    EXPORT_SYMBOL( storage_get_u8 ),
    EXPORT_SYMBOL( storage_set_blob ),
    EXPORT_SYMBOL( storage_set_str ),
    EXPORT_SYMBOL( storage_set_u16 ),
    EXPORT_SYMBOL( storage_set_u32 ),
    EXPORT_SYMBOL( storage_set_u8 ),
    EXPORT_SYMBOL( strcasecmp ),
    EXPORT_SYMBOL( strcat ),
    EXPORT_SYMBOL( strchr ),
    EXPORT_SYMBOL( strcmp ),
    EXPORT_SYMBOL( strcpy ),
    EXPORT_SYMBOL( strcspn ),
    EXPORT_SYMBOL( strdup ),
    EXPORT_SYMBOL( strftime ),
    EXPORT_SYMBOL( strlcat ),
    EXPORT_SYMBOL( strlcpy ),
    EXPORT_SYMBOL( strlen ),
    EXPORT_SYMBOL( strlwr ),
    EXPORT_SYMBOL( strncasecmp ),
    EXPORT_SYMBOL( strncat ),
    EXPORT_SYMBOL( strncmp ),
    EXPORT_SYMBOL( strncpy ),
    EXPORT_SYMBOL( strndup ),
    EXPORT_SYMBOL( strnlen ),
    EXPORT_SYMBOL( strrchr ),
    EXPORT_SYMBOL( strstr ),
    EXPORT_SYMBOL( strtod ),
    EXPORT_SYMBOL( strtof ),
    EXPORT_SYMBOL( strtol ),
    EXPORT_SYMBOL( strupr ),
    EXPORT_SYMBOL( subconsole_cmd_free ),
    EXPORT_SYMBOL( subconsole_cmd_init ),
    EXPORT_SYMBOL( subconsole_cmd_register ),
    EXPORT_SYMBOL( subconsole_cmd_run ),
    EXPORT_SYMBOL_PTR( vault ),
    EXPORT_SYMBOL( vault_refresh ),
    EXPORT_SYMBOL( vault_sem_give ),
    EXPORT_SYMBOL( vault_sem_take ),
    EXPORT_SYMBOL( xQueueCreateCountingSemaphore ),
    EXPORT_SYMBOL( xQueueCreateMutex ),
    EXPORT_SYMBOL( xQueueGenericCreate ),
    EXPORT_SYMBOL( xQueueGenericReceive ),
    EXPORT_SYMBOL( xQueueGenericReset ),
    EXPORT_SYMBOL( xQueueGenericSend ),
    EXPORT_SYMBOL( xQueueGenericSendFromISR ),
    EXPORT_SYMBOL( xQueueGetMutexHolder ),
    EXPORT_SYMBOL( xQueueGiveFromISR ),
    EXPORT_SYMBOL( xQueueGiveMutexRecursive ),
    EXPORT_SYMBOL( xQueueReceiveFromISR ),
    EXPORT_SYMBOL( xQueueTakeMutexRecursive ),
};

const ELFLoaderEnv_t env = { exports, sizeof(exports) / sizeof(*exports) };

#endif
