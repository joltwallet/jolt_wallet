#ifndef JOLT_LANG_H__
#define JOLT_LANG_H__

#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"
#include "sdkconfig.h"

/* Only append languages to this list (aside from JOLT_LANG_LAST_LANG) */
enum {
    JOLT_LANG_ENGLISH = 0,
    JOLT_LANG_SPANISH,
    JOLT_LANG_LAST_LANG,
};
typedef uint8_t jolt_lang_t;

/* When listing languages, order to display them */
extern const jolt_lang_t jolt_lang_order[JOLT_LANG_LAST_LANG];

typedef enum jolt_text_id_t {
    /* Error Handling */
    JOLT_TEXT_MISSING_STRING = 0,

    /* Main Menuing and General Options */
    JOLT_TEXT_MAIN_MENU_TITLE, /* main menu title (the homescreen) */
    JOLT_TEXT_SETTINGS, /* Menu option to change device settings */
    JOLT_TEXT_WIFI,
    JOLT_TEXT_BLUETOOTH,

    /* Confirmation */
    JOLT_TEXT_YES,
    JOLT_TEXT_NO,

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

    /* Bluetooth Options */
    JOLT_TEXT_BLUETOOTH_ENABLE,
    JOLT_TEXT_PAIR,
    JOLT_TEXT_TEMP_PAIR,
    JOLT_TEXT_UNBONDS,
    JOLT_TEXT_BROADCASTING,

    /* WiFi Options */
    JOLT_TEXT_WIFI_ENABLE,
    JOLT_TEXT_WIFI_DETAILS,

    /* Mnemonic Restore */
    JOLT_TEXT_MNEMONIC_RESTORE,
    JOLT_TEXT_RESTORE,
    JOLT_TEXT_BEGIN_MNEMONIC_RESTORE,
    JOLT_TEXT_ENTER_MNEMONIC_WORD,

    /* Error */
    JOLT_TEXT_ERROR,
    JOLT_TEXT_ERROR_CODE,

    /* Used for allocating resources. Not actually a string */
    JOLT_TEXT_LAST_STR,
} jolt_text_id_t;


const char *gettext( jolt_text_id_t id );

const char *getlangname( jolt_lang_t lang);

bool jolt_lang_set( jolt_lang_t lang );

bool jolt_lang_available( jolt_lang_t lang );

#if CONFIG_JOLT_LANG_ENGLISH_EN
extern const char *jolt_lang_english[JOLT_TEXT_LAST_STR];
extern lv_font_t *jolt_lang_english_font;
extern uint8_t jolt_lang_english_n_ext;
extern lv_font_t *jolt_lang_english_ext_fonts[];
#endif

#if CONFIG_JOLT_LANG_SPANISH_EN
extern const char *jolt_lang_spanish[JOLT_TEXT_LAST_STR];
extern lv_font_t *jolt_lang_spanish_font;
extern uint8_t jolt_lang_spanish_n_ext;
extern lv_font_t *jolt_lang_spanish_ext_fonts[];
#endif

#endif
