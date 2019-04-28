#include "jolt_gui/jolt_gui.h"
#include "esp_log.h"
#include "esp_spiffs.h"


static const char TAG[] = "gui_storage";

void menu_storage_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        uint32_t tot, used;
        uint8_t percentage;
        char subtitle[50];

        esp_spiffs_info(NULL, &tot, &used);
        percentage = (used*100)/tot;

        snprintf(subtitle, sizeof(subtitle), "%dKB Free / %dKB Total", used/1024, tot/1024);

        jolt_gui_obj_t *scr;
        scr = jolt_gui_scr_loadingbar_create( gettext(JOLT_TEXT_STORAGE) );
        if( NULL == scr ){
            goto exit;
        }
        jolt_gui_scr_loadingbar_update(scr, NULL, subtitle, percentage);
        jolt_gui_scr_set_event_cb(scr, jolt_gui_event_del);
    }

exit:
    return;
}
