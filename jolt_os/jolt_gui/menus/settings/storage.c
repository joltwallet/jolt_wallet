#include "jolt_gui/jolt_gui.h"
#include "esp_log.h"
#include "syscore/filesystem.h"
#include "submenus.h"
#include "jolt_helpers.h"


static const char TAG[] = "gui_storage";

static void menu_storage_callback(lv_obj_t *obj, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        /* Create Storage Details menu. */
        menu_storage_details_create(obj, jolt_gui_event.short_clicked);

    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del(obj);
    }
}

void menu_storage_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        uint32_t total, used;
        uint8_t percentage;
        char subtitle[50];
        char used_buf[10];
        char total_buf[10];

        jolt_fs_info(&total, &used);
        percentage = (used*100)/total;
        jolt_bytes_to_hstr(used_buf, sizeof(used_buf), used, 0);
        jolt_bytes_to_hstr(total_buf, sizeof(total_buf), total, 0);

        snprintf(subtitle, sizeof(subtitle), gettext(JOLT_TEXT_STORAGE_USAGE), used_buf, total_buf);
        ESP_LOGD(TAG, "Storage subtitle: %s", subtitle);

        jolt_gui_obj_t *scr;
        scr = jolt_gui_scr_loadingbar_create( gettext(JOLT_TEXT_STORAGE) );
        if( NULL == scr ) goto exit;
        jolt_gui_scr_loadingbar_update(scr, NULL, subtitle, percentage);
        jolt_gui_scr_set_event_cb(scr, menu_storage_callback);
    }

exit:
    return;
}
