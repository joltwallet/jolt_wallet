
#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

static const char TAG[] = "gui_debug";

/* Prints out debug information about an object */
void jolt_gui_debug_obj_print( jolt_gui_obj_t *obj )
{
    if( NULL == obj ) { return; }
    jolt_gui_obj_t *parent = NULL;
    lv_obj_type_t parent_type, obj_type;
    JOLT_GUI_CTX
    {
        parent = lv_obj_get_parent( obj );
        lv_obj_get_type( parent, &parent_type );
        lv_obj_get_type( obj, &obj_type );
    }
    ESP_LOGD( TAG,
              "\n###################\n"
              "LVGL Object at %p\n"
              "Object Type: %s\n"
              "Object Size: w:%d, h:%d\n"
              "Parent at %p\n"
              "Parent Type: %s\n"
              "###################\n",
              obj, obj_type.type[0], lv_obj_get_width( obj ), lv_obj_get_height( obj ), parent, parent_type.type[0] );
}

const char *jolt_gui_event_to_str( jolt_gui_event_t event )
{
    if( event == jolt_gui_event.pressed )
        return "pressed";
    else if( event == jolt_gui_event.pressing )
        return "pressing";
    else if( event == jolt_gui_event.press_list )
        return "press_list";
    else if( event == jolt_gui_event.short_clicked )
        return "short_clicked";
    else if( event == jolt_gui_event.long_pressed )
        return "long_pressed";
    else if( event == jolt_gui_event.long_pressed_repeat )
        return "long_pressed_repeat";
    else if( event == jolt_gui_event.clicked )
        return "clicked";
    else if( event == jolt_gui_event.released )
        return "released";
    else if( event == jolt_gui_event.drag_begin )
        return "drag_begin";
    else if( event == jolt_gui_event.drag_end )
        return "drag_end";
    else if( event == jolt_gui_event.drag_throw_begin )
        return "throw begin";
    else if( event == jolt_gui_event.focused )
        return "focused";
    else if( event == jolt_gui_event.defocused )
        return "defocused";
    else if( event == jolt_gui_event.value_changed )
        return "value_changed";
    else if( event == jolt_gui_event.insert )
        return "insert";
    else if( event == jolt_gui_event.selected )
        return "selected";
    else if( event == jolt_gui_event.refresh )
        return "refresh";
    else if( event == jolt_gui_event.apply )
        return "apply";
    else if( event == jolt_gui_event.cancel )
        return "cancel";
    else if( event == jolt_gui_event.delete )
        return "delete";
    else
        return "Unknown";
}
