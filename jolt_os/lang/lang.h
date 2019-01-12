#ifndef JOLT_LANG_H__
#define JOLT_LANG_H__

#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"
#include "sdkconfig.h"

typedef enum jolt_lang_t {
    JOLT_LANG_ENGLISH = 0,
} jolt_lang_t;

typedef enum jolt_text_id_t {
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
    JOLT_TEXT_BRIGHTNESS,
    JOLT_TEXT_FACTORY_RESET,

    /* Language Names */
    JOLT_TEXT_LANGUAGE,
    JOLT_TEXT_ENGLISH,
    JOLT_TEXT_SPANISH,
    
    /* App Launcher */
    JOLT_TEXT_PRELOAD_LAUNCHING,

    /* Used for allocating resources. Not actually a string */
    JOLT_TEXT_LAST_STR,
} jolt_text_id_t;


const char *gettext( jolt_text_id_t id );

bool jolt_lang_set( jolt_lang_t lang );

bool jolt_lang_available( jolt_lang_t lang );

#if CONFIG_JOLT_LANG_ENGLISH_EN
extern const lv_font_t *jolt_lang_english_font;
extern const char *jolt_lang_english[JOLT_TEXT_LAST_STR];
#endif

#endif
