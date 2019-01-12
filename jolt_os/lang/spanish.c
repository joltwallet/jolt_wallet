#include "jolt_gui/jolt_gui.h"

#if CONFIG_JOLT_LANG_SPANISH_EN

/* todo: accent support */
const lv_font_t *jolt_lang_spanish_font = &lv_font_pixelmix_7;

const char *jolt_lang_spanish[JOLT_TEXT_LAST_STR] = {
    /* Error Handling */
    "<missing_string>",

    /* Main Menuing and General Options */
    "Inicio", /* main menu title (the homescreen) */
    "Ajustes", /* Menu option to change device settings */
    "WiFi",
    "Bluetooth",

    /* Confirmation */
    "Sí",
    "No",

    /* Settings */
    "Brillo pantalla",
    "Brillo",
    "Restablecimiento de fábrica",

    /* Language Names */
    "Idioma",
    "Inglés",
    "Español",

    /* App Launcher */
    "Lanzamiento", /* Label for spinning preloader */
};


#endif
