#ifndef __JOLT_LIB_EXPORTS_H__
#define __JOLT_LIB_EXPORTS_H__

#include "bipmnemonic.h"
#include "esp_console.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "menu8g2.h"
#include "sodium.h"
#include "sodium/crypto_verify_32.h"
#include "sodium/private/curve25519_ref10.h"
#include "u8g2.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"
#include "qrcode.h"
#include "nano_rest.h"

#include "console.h"
#include "globals.h"
#include "vault.h"
#include "helpers.h"
#include "gui/gui.h"
#include "gui/graphics.h"
#include "gui/loading.h"
#include "gui/statusbar.h"
#include "gui/confirmation.h"
#include "gui/qr.h"
#include "syscore/filesystem.h"
#include "hal/storage.h"
//    EXPORT_SYMBOL(  ),
//
const ELFLoaderSymbol_t exports[] = {
    EXPORT_SYMBOL( bm_entropy256 ),
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
    EXPORT_SYMBOL( display_qr_center ),
    EXPORT_SYMBOL( esp_log_timestamp ),
    EXPORT_SYMBOL( esp_log_write ),
    EXPORT_SYMBOL( esp_restart ),
    EXPORT_SYMBOL( free ),
    EXPORT_SYMBOL( get_display_brightness ),
    EXPORT_SYMBOL( hd_node_copy ),
    EXPORT_SYMBOL( hd_node_iterate ),
    EXPORT_SYMBOL( heap_caps_calloc ),
    EXPORT_SYMBOL( linenoise ),
    EXPORT_SYMBOL( loading_disable ),
    EXPORT_SYMBOL( loading_enable ),
    EXPORT_SYMBOL( malloc ),
    EXPORT_SYMBOL( mbedtls_mpi_cmp_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_copy ),
    EXPORT_SYMBOL( mbedtls_mpi_free ),
    EXPORT_SYMBOL( mbedtls_mpi_init ),
    EXPORT_SYMBOL( mbedtls_mpi_write_binary ),
    EXPORT_SYMBOL( mbedtls_mpi_write_string ),
    EXPORT_SYMBOL( memchr ),
    EXPORT_SYMBOL( memcmp ),
    EXPORT_SYMBOL( memcpy ),
    EXPORT_SYMBOL( memmove ),
    EXPORT_SYMBOL( memset ),
    EXPORT_SYMBOL_PTR( menu ),
    EXPORT_SYMBOL( menu8g2_buf_header ),
    EXPORT_SYMBOL( menu8g2_copy ),
    EXPORT_SYMBOL( menu8g2_create_simple ),
    EXPORT_SYMBOL( menu8g2_create_vertical_element_menu ),
    EXPORT_SYMBOL( menu8g2_create_vertical_menu ),
    EXPORT_SYMBOL( menu8g2_display_text_title ),
    EXPORT_SYMBOL( menu8g2_draw_str ),
    EXPORT_SYMBOL( menu8g2_elements_free ),
    EXPORT_SYMBOL( menu8g2_elements_init ),
    EXPORT_SYMBOL( menu8g2_get_center_x ),
    EXPORT_SYMBOL( menu8g2_init ),
    EXPORT_SYMBOL( menu8g2_set_element ),
    EXPORT_SYMBOL( menu8g2_word_wrap ),
    EXPORT_SYMBOL( menu_confirm_action ),
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
    EXPORT_SYMBOL_PTR( statusbar_draw_enable ),
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
    EXPORT_SYMBOL( u8g2_ClearBuffer ),
    EXPORT_SYMBOL( u8g2_ClearDisplay ),
    EXPORT_SYMBOL( u8g2_DrawBox ),
    EXPORT_SYMBOL( u8g2_DrawFrame ),
    EXPORT_SYMBOL( u8g2_DrawGlyph ),
    EXPORT_SYMBOL( u8g2_DrawHLine ),
    EXPORT_SYMBOL( u8g2_DrawHVLine ),
    EXPORT_SYMBOL( u8g2_DrawStr ),
    EXPORT_SYMBOL( u8g2_DrawXBM ),
    EXPORT_SYMBOL( u8g2_FirstPage ),
    EXPORT_SYMBOL( u8g2_GetGlyphWidth ),
    EXPORT_SYMBOL( u8g2_GetStrWidth ),
    EXPORT_SYMBOL( u8g2_IsIntersection ),
    EXPORT_SYMBOL( u8g2_NextPage ),
    EXPORT_SYMBOL( u8g2_SendBuffer ),
    EXPORT_SYMBOL( u8g2_SetBufferCurrTileRow ),
    EXPORT_SYMBOL( u8g2_SetDrawColor ),
    EXPORT_SYMBOL( u8g2_SetFont ),
    EXPORT_SYMBOL( u8g2_SetFontPosBaseline ),
    EXPORT_SYMBOL( u8g2_SetupBuffer ),
    EXPORT_SYMBOL( u8g2_font_profont12_tf ),
    EXPORT_SYMBOL( u8g2_font_profont17_tf ),
    EXPORT_SYMBOL( u8x8_SetContrast ),
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

#endif
