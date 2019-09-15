//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/bg.h"

static const char TAG[] = "jolt_gui/menus/settings/factory_reset.c";

static int factory_reset( jolt_bg_job_t *job )
{
    ESP_LOGD( TAG, "Performing factory reset" );
    storage_factory_reset( true );
    return 0;
}

static void factory_reset_no_cb( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event || jolt_gui_event.cancel == event ) { jolt_gui_scr_del( btn ); }
}

static void factory_reset_yes_cb( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        ESP_LOGD( TAG, "Creating preloading factory reset screen" );
        jolt_gui_scr_preloading_create( "Factory Reset", "Erasing..." );

        esp_err_t err;
        ESP_LOGD( TAG, "Creating factory reset BG job" );
        err = jolt_bg_create( factory_reset, NULL, NULL );
        if( ESP_OK != err ) { esp_restart(); }
    }
}

void menu_factory_reset_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_scr_yesno_create( gettext( JOLT_TEXT_FACTORY_RESET ), factory_reset_yes_cb, factory_reset_no_cb );
    }
}
