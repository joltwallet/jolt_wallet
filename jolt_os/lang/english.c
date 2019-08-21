#include "jolt_gui/jolt_gui.h"

#if CONFIG_JOLT_LANG_ENGLISH_EN

lv_font_t *jolt_lang_english_font = JOLT_GUI_FONT_DEFAULT;

const char *jolt_lang_english[JOLT_TEXT_LAST_STR] = {
    /* Error Handling */
    "<missing_string>",

    /* Main Menuing and General Options */
    "Home", /* main menu title (the homescreen) */
    "Settings", /* Menu option to change device settings */
    "WiFi",
    "Bluetooth",
    "continue",

    /* Confirmation */
    "Yes",
    "No",

    /* Settings */
    "Screen Brightness",
    "Touch Brightness",
    "Brightness",
    "Factory Reset",
    "Storage",

    /* Language Names */
    "Language",
    "English",
    "Spanish",

    /* App Launcher */
    "Launching", /* Label for spinning preloader */
    "Application update required to run application",
    "JoltOS update required to run application.",

    /* Bluetooth Options */
    "Bluetooth Enable",
    "Pair",
    "Temporary Pair",
    "Forget Devices",
    "Broadcasting",

    /* Storage Options */
    "%dKB Used / %dKB Total",

    /* WiFi Options */
    "WiFi Enable",
    "WiFi Details",

    /* Mnemonic Restore */
    "Mnemonic Restore",
    "Restore",
    "Begin mnemonic restore?",
    "Enter Mnemonic Word",

    /* Vault */
    "PIN",
    "Checking PIN",
    "Incorrect PIN",
    "Unlocking",

    /* App Key */
    "WARNING: This will perform a factory reset.\nSet app public key to:\n?",
    "Set App Key",

    /* Error */
    "Error",
    "Error Code",

    /* Data Transfer */
    "Connecting...",
    "Transferring...",
    "Installing...",
    "Saving...",
    "Processing...",

    /* First Boot */
    "PIN Setup",
    "PIN Verify",
    "Pin Mismatch! Please try again.",
    "Write Down Mnemonic!",
    "First Startup",
    "Welcome to Jolt, please backup the following secret mnemonic."

    /* Misc */
    "...",
};
#endif
