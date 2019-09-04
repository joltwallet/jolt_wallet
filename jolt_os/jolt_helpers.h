/**
 * @file jolt_helpers.h
 * @brief Misc helper functions used in JoltOS
 * @author Brian Pugh
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_HELPERS_H__
#define __JOLT_HELPERS_H__

#include "jolttypes.h"
#include "esp_err.h"
#include "bipmnemonic.h"
#include "lvgl/lvgl.h"
#include "vault.h"

#define XJOLT_ADC1(x) ADC1_GPIO ## x ## _CHANNEL
/**
 * @brief Converts integer GPIO pin from Kconfig to ADC pin definition for ADC1.
 */
#define JOLT_ADC1(x) XJOLT_ADC1(x)
/**
 * @brief ADC pin to read battery voltage from
 */
#define JOLT_ADC1_VBATT JOLT_ADC1(CONFIG_JOLT_VBATT_SENSE_PIN)

/**
 * @brief Derivation path purpose for JoltOS settings
 */
#define JOLT_OS_DERIVATION_PURPOSE ((uint32_t) BM_HARDENED | 44)

/**
 * @brief Derivation path coin_type (JOLT) for JoltOS settings
 */
#define JOLT_OS_DERIVATION_PATH ((uint32_t) BM_HARDENED | 0x4A4F4C54)

/**
 * @brief free non-null pointer.
 */
#define SAFE_FREE(x) do{if(NULL != x) {free(x); x = NULL;}}while(0)

/**
 * @brief fclose non-null pointer.
 */
#define SAFE_CLOSE(x) do{if(NULL != x) {fclose(x); x = NULL;}}while(0)

/**
 * @brief Set return_code and goto exit
 */
#define EXIT(ret) do{ return_code = ret; goto exit; }while(0)

/**
 * @brief Set return_code, print statement, and goto exit.
 */
#define EXIT_PRINT(ret, ...) do{ return_code = ret; printf(__VA_ARGS__); goto exit; }while(0)

/**
 * @brief goto exit if pointer is NULL.
 */
#define EXIT_IF_NULL(x) do{ if( NULL == x ) goto exit; }while(0)

/**
 * @brief Easy reference to a NULL TERMINATOR character '\0'
 */
extern const char NULL_TERM;

extern const char *EMPTY_STR;

/**
 * @brief Derivation BIP32 key for JoltOS settings 
 */
static const char JOLT_OS_DERIVATION_BIP32_KEY[] = "JOLT_OS";
/**
 * @brief Derivation BIP32 passphrase for JoltOS settings
 */
static const char JOLT_OS_DERIVATION_PASSPHRASE[] = "";

/**
 * @brief Get a random number of bytes of cryptographically secure entropy.
 *
 * Uses as many RNG sources as possible.
 *
 * @param[out] buf Store random bytes.
 * @param[in] n_bytes Number of random bytes to generate.
 */
void jolt_get_random(uint8_t *buf, uint8_t n_bytes);

/**
 * @brief Fisher Yates random shuffling for an array of uint8_t
 * @param[in,out] arr pointer to array to shuffle
 * @param[in] arr_len number of elements in arr
 * @return
 */
void shuffle_arr(uint8_t *arr, int arr_len);

/**
 * @brief Allocates an array of character arrays
 *
 * Does not allocate space for each string array.
 *
 * @param[in] n Number of character pointers to allocate.
 * @return pointer to char* array
 */
char **jolt_h_malloc_char_array(int n);

/**
 * @brief Frees all individual strings and the pointer array itself
 * @param[in] arr pointer to array of allocated strings.
 * @param[in] n length of pointer array
 */
void jolt_h_free_char_array(char **arr, int n);

/**
 * @brief Checks if the str ends with suffix
 * @param[in] str NULL-terminated string to search
 * @param[in] suffix NULL-terminated suffix to check
 * @return True if str ends with suffix; false otherwise.
 */
bool jolt_h_strcmp_suffix( const char *str, const char *suffix);

/**
 * @brief Refresh the home-screen if the string ends in ".jelf"
 *
 * Convenience function to refresh the home-screen when uploading files that
 * are visible in the home screen.
 *
 * @param[in] NULL-terminated string representing a new file.
 */
void jolt_h_fn_home_refresh(const char *str);

/**
 * @brief Dummy-node used for prompting user pin for system settings/action.
 * @param[in] fail_cb Callback on PIN failure
 * @param[in] success_cb Callback on PIN success
 * @param[in] param Free param passed to callbacks
 */
void jolt_h_settings_vault_set(vault_cb_t fail_cb, vault_cb_t success_cb, void *param);

/**
 * @brief If it's a patch file, try to apply it. Will remove the patch file upon
 * success or failure.
 */
void jolt_h_apply_patch(const char *filename);

/**
 * @brief suspend logging.
 * Can be called multiple times. `jolt_resume_logging` must be called the same 
 * number of times as `jolt_suspend_logging` was called to resume.
 */
void jolt_suspend_logging();

/**
 * @brief resume logging.
 * Can be called multiple times. `jolt_resume_logging` must be called the same 
 * number of times as `jolt_suspend_logging` was called to resume.
 */
void jolt_resume_logging();

#include <driver/uart.h>
#include "hal/radio/bluetooth.h"
/* Log something to uart if BLE is the stdin */
#define BLE_UART_LOG(format, ...) \
    if( stdin == ble_stdin){ \
        char buf[1512]; \
        snprintf(buf, sizeof(buf), format, __VA_ARGS__); \
        uart_write_bytes(UART_NUM_0, buf, strlen(buf)); \
    }

#define BLE_UART_LOG_STR(buf) \
    if(stdin == ble_stdin){ \
        uart_write_bytes(UART_NUM_0, buf, strlen(buf)); \
    }

#define BLE_UART_LOG_BUF(buf, buf_len) \
    if(stdin == ble_stdin){ \
        uart_write_bytes(UART_NUM_0, buf, buf_len); \
    }

#define BLE_UART_LOGD(format, ...)      do{if(LOG_LOCAL_LEVEL >= 4) BLE_UART_LOG(format, __VA_ARGS__);}while(0)
#define BLE_UART_LOGD_STR(buf)          do{if(LOG_LOCAL_LEVEL >= 4) BLE_UART_LOG_STR(buf);}while(0)
#define BLE_UART_LOGD_BUF(buf, buf_len) do{if(LOG_LOCAL_LEVEL >= 4) BLE_UART_LOG_BUF(buf, buf_len);}while(0)

#define BLE_UART_LOGI(format, ...)      do{if(LOG_LOCAL_LEVEL >= 3) BLE_UART_LOG(format, __VA_ARGS__);}while(0)
#define BLE_UART_LOGI_STR(buf)          do{if(LOG_LOCAL_LEVEL >= 3) BLE_UART_LOG_STR(buf);}while(0)
#define BLE_UART_LOGI_BUF(buf, buf_len) do{if(LOG_LOCAL_LEVEL >= 3) BLE_UART_LOG_BUF(buf, buf_len);}while(0)

#define BLE_UART_LOGW(format, ...)      do{if(LOG_LOCAL_LEVEL >= 2) BLE_UART_LOG(format, __VA_ARGS__);}while(0)
#define BLE_UART_LOGW_STR(buf)          do{if(LOG_LOCAL_LEVEL >= 2) BLE_UART_LOG_STR(buf);}while(0)
#define BLE_UART_LOGW_BUF(buf, buf_len) do{if(LOG_LOCAL_LEVEL >= 2) BLE_UART_LOG_BUF(buf, buf_len);}while(0)

#define BLE_UART_LOGE(format, ...)      do{if(LOG_LOCAL_LEVEL >= 1) BLE_UART_LOG(format, __VA_ARGS__);}while(0)
#define BLE_UART_LOGE_STR(buf)          do{if(LOG_LOCAL_LEVEL >= 1) BLE_UART_LOG_STR(buf);}while(0)
#define BLE_UART_LOGE_BUF(buf, buf_len) do{if(LOG_LOCAL_LEVEL >= 1) BLE_UART_LOG_BUF(buf, buf_len);}while(0)

#endif
