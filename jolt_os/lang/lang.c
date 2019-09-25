#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/jolt_gui_symbols.h"

static const char TAG[] = "lang.c";

typedef uint64_t jolt_lang_avail_t;

/* Pointer to the list of strings for the current language */
static const char **lang_pack = NULL;

/* These can be changed to any order */
const jolt_lang_t jolt_lang_order[JOLT_LANG_LAST_LANG] = {
        JOLT_LANG_ENGLISH,
        JOLT_LANG_SPANISH,
};

/********************
 * Public Functions *
 ********************/

/* Returns specified text in current language */
const char *gettext( jolt_text_id_t id )
{
    if( NULL == lang_pack ) { jolt_lang_set( CONFIG_JOLT_LANG_DEFAULT ); }
    return lang_pack[id];
}

/* Given a language id, return the string for it */
const char *getlangname( jolt_lang_t lang )
{
    jolt_text_id_t id = JOLT_TEXT_MISSING_STRING;
    switch( lang ) {
        case JOLT_LANG_ENGLISH: id = JOLT_TEXT_ENGLISH; break;
        case JOLT_LANG_SPANISH: id = JOLT_TEXT_SPANISH; break;
    }
    return gettext( id );
}

/* Returns true if language successfully set, false if language is not available */
bool jolt_lang_set( jolt_lang_t lang )
{
    if( !jolt_lang_available( lang ) ) { return false; }
    lv_font_t *font = NULL; /* base font to add extension fonts to */

    jolt_lang_t current_lang;
    storage_get_u8( &current_lang, "user", "lang", CONFIG_JOLT_LANG_DEFAULT );

    /* Save the font and restart */
    if( current_lang != lang ) {
        ESP_LOGI( TAG, "Different language detected; rebooting..." );
        storage_set_u8( lang, "user", "lang" );
        esp_restart();
    }

#if CONFIG_JOLT_LANG_ENGLISH_EN
    else if( JOLT_LANG_ENGLISH == lang ) {
        lang_pack = jolt_lang_english;
        font      = jolt_lang_english_font;
    }
#endif

#if CONFIG_JOLT_LANG_SPANISH_EN
    else if( JOLT_LANG_SPANISH == lang ) {
        lang_pack = jolt_lang_spanish;
        font      = jolt_lang_spanish_font;
    }
#endif

    lv_theme_t *theme = jolt_gui_theme_init( 0, font );
    lv_theme_set_current( theme );

    /* For some reason, LVGL was having issues wth fullscreen refreshes without
     * this dummy object taking up the entire screen. To be investigated */
    lv_obj_t *dummy = RESTART_IF_NULL( lv_cont_create( lv_scr_act(), NULL ) );
    lv_obj_set_size( dummy, LV_HOR_RES_MAX, LV_VER_RES_MAX );

    return true;
}

/* Returns True if the queried language is available */
bool jolt_lang_available( jolt_lang_t lang )
{
#define SHIFT( x ) ( 1 << x ) |
    static jolt_lang_avail_t available_lang =
#if CONFIG_JOLT_LANG_ENGLISH_EN
            SHIFT( JOLT_LANG_ENGLISH )
#endif
#if CONFIG_JOLT_LANG_SPANISH_EN
                    SHIFT( JOLT_LANG_SPANISH )
#endif
                            0;
#undef SHIFT

    return ( 1 << lang ) & available_lang;
}
