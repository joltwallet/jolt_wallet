/**
 * @file lang.h
 * @brief Jolt internationalization
 * @author Brian Pugh
 */

#ifndef JOLT_LANG_H__
#define JOLT_LANG_H__

#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"
#include "sdkconfig.h"

/**
 * @brief Langues supported by JoltOS.
 *
 * Only append languages to this list (aside from JOLT_LANG_LAST_LANG)
 */
enum {
    JOLT_LANG_ENGLISH = 0,
    JOLT_LANG_SPANISH,
    JOLT_LANG_LAST_LANG, /**< Number of Langues */
};
typedef uint8_t jolt_lang_t;

/**
 * @brief Order to display languages
 */
extern const jolt_lang_t jolt_lang_order[JOLT_LANG_LAST_LANG];

/**
 * @brief Human-readable indexs into language pack strings.
 */
typedef enum jolt_text_id_t {
    /* Error Handling */
    JOLT_TEXT_MISSING_STRING = 0,

    /* Main Menuing and General Options */
    JOLT_TEXT_MAIN_MENU_TITLE, /* main menu title (the homescreen) */
    JOLT_TEXT_SETTINGS,        /* Menu option to change device settings */
    JOLT_TEXT_WIFI,
    JOLT_TEXT_BLUETOOTH,
    JOLT_TEXT_CONTINUE,        /* lowercase */

    /* Confirmation */
    JOLT_TEXT_YES,             /* First letter capitlaized */
    JOLT_TEXT_NO,              /* First letter capitlaized */

    /* Settings */
    JOLT_TEXT_SCREEN_BRIGHTNESS,
    JOLT_TEXT_LED_BRIGHTNESS,
    JOLT_TEXT_BRIGHTNESS,
    JOLT_TEXT_FACTORY_RESET,
    JOLT_TEXT_STORAGE,

    /* Language Names */
    JOLT_TEXT_LANGUAGE,
    JOLT_TEXT_ENGLISH,
    JOLT_TEXT_SPANISH,
    
    /* App Launcher */
    JOLT_TEXT_PRELOAD_LAUNCHING,
    JOLT_TEXT_LAUNCH_APP_OUT_OF_DATE,
    JOLT_TEXT_LAUNCH_JOLTOS_OUT_OF_DATE,

    /* Bluetooth Options */
    JOLT_TEXT_BLUETOOTH_ENABLE,
    JOLT_TEXT_PAIR,
    JOLT_TEXT_TEMP_PAIR,
    JOLT_TEXT_UNBONDS,
    JOLT_TEXT_BROADCASTING,

    /* WiFi Options */
    JOLT_TEXT_WIFI_ENABLE,
    JOLT_TEXT_WIFI_DETAILS,

    /* Storage Options */
    JOLT_TEXT_STORAGE_USAGE,

    /* Mnemonic Restore */
    JOLT_TEXT_MNEMONIC_RESTORE,
    JOLT_TEXT_RESTORE,
    JOLT_TEXT_BEGIN_MNEMONIC_RESTORE,
    JOLT_TEXT_ENTER_MNEMONIC_WORD,

    /* Vault */
    JOLT_TEXT_PIN,
    JOLT_TEXT_CHECKING_PIN,
    JOLT_TEXT_INCORRECT_PIN,
    JOLT_TEXT_UNLOCKING,

    /* App Ket */
    JOLT_TEXT_APP_KEY_SET_CONFIRMATION,
    JOLT_TEXT_APP_KEY_SET_CONFIRMATION_TITLE,

    /* Error */
    JOLT_TEXT_ERROR,
    JOLT_TEXT_ERROR_CODE,

    /* Data Transfer */
    JOLT_TEXT_CONNECTING,
    JOLT_TEXT_TRANSFERRING,
    JOLT_TEXT_INSTALLING,
    JOLT_TEXT_SAVING,
    JOLT_TEXT_PROCESSING,

    /* First Boot */
    JOLT_TEXT_PIN_SETUP,
    JOLT_TEXT_PIN_VERIFY,
    JOLT_TEXT_PIN_MISMATCH,
    JOLT_TEXT_BACKUP_MNEMONIC,
    JOLT_TEXT_FIRST_STARTUP,
    JOLT_TEXT_WELCOME_DIALOG_0,

    /* Misc */
    JOLT_TEXT_ELLIPSIS,

    /* Used for allocating resources. Not actually a string */
    JOLT_TEXT_LAST_STR,
} jolt_text_id_t;

/**
 * @brief Get the specified character string in the current language
 * @param[in] id index into language-pack strings
 * @return NULL-terminated character string.
 */
const char *gettext( jolt_text_id_t id );

/**
 * @brief Get the specified language name in current language 
 * @param[in] lang a JOLT_LANG_*
 * @return NULL-terminated character string.
 */
const char *getlangname( jolt_lang_t lang);

/**
 * @brief Sets system language. If language is different than saved; it will save and reset.
 *
 * @param[in] lang language to set
 * @return True on success; false if language was not compiled.
 */
bool jolt_lang_set( jolt_lang_t lang );

/**
 * @brief Checks if specified language was compiled into JoltOS
 * @param[in] lang Language to check availability.
 * @return True if language is available; false otherwise
 */
bool jolt_lang_available( jolt_lang_t lang );

#if CONFIG_JOLT_LANG_ENGLISH_EN
extern const char *jolt_lang_english[JOLT_TEXT_LAST_STR];
extern lv_font_t *jolt_lang_english_font;
#endif

#if CONFIG_JOLT_LANG_SPANISH_EN
extern const char *jolt_lang_spanish[JOLT_TEXT_LAST_STR];
extern lv_font_t *jolt_lang_spanish_font;
#endif

#endif
