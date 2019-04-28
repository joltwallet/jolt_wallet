#if JOLT_GUI_DEBUG_FUNCTIONS

#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "gui_debug";

/* Prints out debug information about an object */
void jolt_gui_debug_obj_print(jolt_gui_obj_t *obj) {
    if( NULL == obj ) {
        return;
    }
    jolt_gui_obj_t *parent = NULL;
    lv_obj_type_t parent_type, obj_type;
    JOLT_GUI_CTX{
        parent = lv_obj_get_parent(obj);
        lv_obj_get_type(parent, &parent_type);
        lv_obj_get_type(obj, &obj_type);
    }
    ESP_LOGI(TAG,
            "\n###################\n"
            "LVGL Object at %p\n"
            "Object Type: %s\n"
            "Object Size: w:%d, h:%d\n"
            "Parent at %p\n"
            "Parent Type: %s\n"
            "###################\n",
            obj, obj_type.type[0],
            lv_obj_get_width(obj), lv_obj_get_height(obj),
            parent, parent_type.type[0]
            );
}

#endif
