#include "jolt_gui/jolt_gui.h"

#if CONFIG_JOLT_LANG_ENGLISH_EN

const lv_font_t *jolt_lang_english_font = &lv_font_pixelmix_7;

const char *jolt_lang_english[JOLT_TEXT_LAST_STR] = {
    /* Main Menuing and General Options */
    "Home", /* main menu title (the homescreen) */
    "Settings", /* Menu option to change device settings */
    "WiFi",
    "Bluetooth",

    /* Confirmation */
    "Yes",
    "No",

    /* Settings */
    "Screen Brightness",
    "Brightness",
    "Factory Reset",

    /* Language Names */
    "English",

    /* App Launcher */
    "Launching", /* Label for spinning preloader */
};


#endif
