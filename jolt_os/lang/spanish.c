#include "jolt_gui/jolt_gui.h"

#if CONFIG_JOLT_LANG_SPANISH_EN

lv_font_t *jolt_lang_spanish_font = JOLT_GUI_FONT_DEFAULT;

const char *jolt_lang_spanish[JOLT_TEXT_LAST_STR] = {
    /* Error Handling */
    "<missing_string>",

    /* Main Menuing and General Options */
    "Inicio",
    "Ajustes",
    "WiFi",
    "Bluetooth",
    "continuar",

    /* Confirmation */
    "Sí",
    "No",

    /* Settings */
    "Brillo pantalla",
    "Toque Brillo",
    "Brillo",
    "Restablecimiento de fábrica",
    "Almacenamiento",

    /* Language Names */
    "Idioma",
    "Inglés",
    "Español",

    /* App Launcher */
    "Lanzamiento",
    "Actualización de la aplicación requerida para ejecutarse.",
    "Actualización JoltOS requerida para ejecutarse.",

    /* Bluetooth Options */
    "Bluetooth habilitado",
    "Par",
    "Par temporal",
    "Olvídate de los dispositivos",
    "Radiodifusión",

    /* WiFi Options */
    "WiFi Habilitado",
    "Detalles de WiFi"

    /* Storage Options */
    "%dKB Usado / %dKB Total",

    /* Mnemonic Restore */
    "restauración mnemónica",
    "Restaurar",
    "Comienza la restauración mnemónica",
    "Introduzca palabra mnemotécnica",

    /* Vault */
    "PIN",
    "Comprobando PIN",
    "PIN incorrecto",
    "Desbloqueo",

    /* App Ket */
    "ADVERTENCIA: Esto realizará un restablecimiento de fábrica.\nEstablezca la clave pública de la aplicación en:",
    "Establecer clave de aplicación",

    /* Error */
    "Error",
    "Código de error",

    /* Data Transfer */
    "Conectando...",
    "Transferencia...",
    "Instalando...",
    "Ahorro...",
    "Tratamiento...",

    /* First Boot */
    "Configuración de PIN",
    "PIN Verificar",
    "PIN no coincide! Inténtalo de nuevo.",
    "Anote mnemotécnico",
    "Primera puesta en marcha",
    "Bienvenido a Jolt, haz una copia de seguridad de la siguiente mnemónica secreta.",

    /* Misc */
    "...",
};
#endif
