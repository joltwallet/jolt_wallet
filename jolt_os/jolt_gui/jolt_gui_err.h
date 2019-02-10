#ifndef JOLT_GUI_SCR_ERR_H__
#define JOLT_GUI_SCR_ERR_H__

typedef enum {
    JOLT_GUI_ERR_UNKNOWN = 1,
    JOLT_GUI_ERR_NOT_IMPLEMENTED,
    JOLT_GUI_ERR_WIFI_NO_DRIVERS,
    JOLT_GUI_ERR_WIFI_OFF,
    JOLT_GUI_ERR_BLUETOOTH_NO_DRIVERS,
    JOLT_GUI_ERR_BLUETOOTH_OFF,
} jolt_gui_err_t;

char *jolt_gui_err_to_str( jolt_gui_err_t err );

lv_obj_t *jolt_gui_scr_err_create( jolt_gui_err_t err );

#endif
