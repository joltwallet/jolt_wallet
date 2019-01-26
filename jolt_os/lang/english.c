#include "jolt_gui/jolt_gui.h"

#if CONFIG_JOLT_LANG_ENGLISH_EN

lv_font_t *jolt_lang_english_font = &lv_font_pixelmix_7;
uint8_t jolt_lang_english_n_ext = 1;
lv_font_t *jolt_lang_english_ext_fonts[] = {
    &lv_font_pixelmix_7_latin_sup,
};

const char *jolt_lang_english[JOLT_TEXT_LAST_STR] = {
    /* Error Handling */
    "<missing_string>",

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
    "Language",
    "English",
    "Spanish",

    /* App Launcher */
    "Launching", /* Label for spinning preloader */

    /* Bluetooth Options */
    "Bluetooth Enable",
};


#endif
