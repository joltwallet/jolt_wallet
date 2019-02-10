#include "jolt_gui.h"

char *jolt_gui_err_to_str( jolt_gui_err_t err ) {
#define STRCASE(name) case name: return #name;
    switch ( err ) {
        default:
        STRCASE( JOLT_GUI_ERR_UNKNOWN )
        STRCASE( JOLT_GUI_ERR_NOT_IMPLEMENTED )
        STRCASE( JOLT_GUI_ERR_WIFI_NO_DRIVERS )
        STRCASE( JOLT_GUI_ERR_WIFI_OFF )
        STRCASE( JOLT_GUI_ERR_BLUETOOTH_NO_DRIVERS )
        STRCASE( JOLT_GUI_ERR_BLUETOOTH_OFF )
    }
#undef STRCASE
}

lv_obj_t *jolt_gui_scr_err_create( jolt_gui_err_t err ) {
    char buf[100];
    snprintf(buf, sizeof(buf), "%s 0x%x\n%s", gettext(JOLT_TEXT_ERROR_CODE), err, jolt_gui_err_to_str(err));
    return jolt_gui_scr_text_create(gettext(JOLT_TEXT_ERROR), buf);
}
